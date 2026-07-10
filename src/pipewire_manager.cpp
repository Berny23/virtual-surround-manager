#include "pipewire_manager.h"

string PipeWireManager::parse_default_sink_name(const char *value) {
    if (!value)
        return {};
    const string json(value);
    const size_t key = json.find("\"name\"");
    if (key == string::npos)
        return json;
    const size_t colon = json.find(':', key);
    if (colon == string::npos)
        return {};
    const size_t start = json.find('"', colon);
    if (start == string::npos)
        return {};
    const size_t end = json.find('"', start + 1);
    if (end == string::npos)
        return {};
    return json.substr(start + 1, end - start - 1);
}

string PipeWireManager::resolve_sink_name(const string &key) const {
    if (key.empty())
        return {};
    for (auto it = sinks.cbegin(); it != sinks.cend(); ++it)
        if (it.value().node_name == key || it.value().serial == key)
            return it.value().node_name;
    return {};
}

bool PipeWireManager::stream_follows_default(uint32_t id, const string &props_target) const {
    // The metadata target takes precedence over the node's own props target
    string target = metadata_targets.value(id, string());
    if (target.empty())
        target = props_target;

    if (target.empty())
        return true;

    const string sink = resolve_sink_name(target);
    if (sink.empty())
        return false;
    return sink == default_sink_name;
}

void PipeWireManager::decide_stream(uint32_t id, const string &props_target) {
    if (!metadata || decided_streams.contains(id))
        return;

    decided_streams.insert(id);

    if (stream_follows_default(id, props_target)) {
        pw_metadata_set_property(metadata, id, PW_KEY_TARGET_OBJECT, "Spa:String:JSON", capture_node_name);
        routed_node_ids.insert(id);
        qDebug("decide_stream: Node %u routed to sink '%s'", id, capture_node_name);
    } else {
        qDebug("decide_stream: Node %u left alone (assigned to another device)", id);
    }
}

void PipeWireManager::on_node_info(void *data, const struct pw_node_info *info) {
    NodeBinding *binding = static_cast<NodeBinding *>(data);
    PipeWireManager *manager = binding->manager;

    if (!info || !info->props)
        return;

    const char *target = spa_dict_lookup(info->props, PW_KEY_TARGET_OBJECT);
    if (!target)
        target = spa_dict_lookup(info->props, "target.node");
    const string props_target = (target && strlen(target) > 0) ? target : "";
    manager->decide_stream(binding->id, props_target);
}

int PipeWireManager::on_metadata_property(void *data,
                                          uint32_t subject, // Node ID
                                          const char *key,
                                          [[maybe_unused]] const char *type,
                                          const char *value) { // nullptr if property removed
    PipeWireManager *manager = static_cast<PipeWireManager *>(data);

    if (!key)
        return 0;

    // Track the default sink so we know which streams follow it
    if (strcmp(key, "default.audio.sink") == 0) {
        const string new_default = parse_default_sink_name(value);
        if (new_default != manager->default_sink_name) {
            manager->default_sink_name = new_default;
            qDebug("on_metadata_property: Default sink is now '%s'", new_default.c_str());
        }
        return 0;
    }

    if (strcmp(key, PW_KEY_TARGET_OBJECT) != 0 && strcmp(key, "target.node") != 0)
        return 0;
    if (subject == manager->playback_node_id)
        return 0;

    // Ignore the echo of our own routing writes
    if (value && strcmp(value, manager->capture_node_name) == 0)
        return 0;

    // WirePlumber uses "-1" to mean "no explicit target, follow the default sink"
    const bool no_target = !value || strlen(value) == 0 || strcmp(value, "-1") == 0;
    if (no_target)
        manager->metadata_targets.remove(subject);
    else
        manager->metadata_targets[subject] = value;

    if (value == nullptr && manager->routed_node_ids.contains(subject)) {
        // Re-assert routing if a stream we virtualize had its target cleared.
        // Only react to a full removal, re-asserting on "-1" could start a write battle with WirePlumber.
        pw_metadata_set_property(manager->metadata, subject, PW_KEY_TARGET_OBJECT, "Spa:String:JSON", manager->capture_node_name);
        qDebug("on_metadata_property: Node %u re-routed to sink '%s'", subject, manager->capture_node_name);
    } else if (!no_target && manager->routed_node_ids.contains(subject)) {
        // The stream was explicitly moved to another device, so it is no longer ours
        manager->routed_node_ids.remove(subject);
        qDebug("on_metadata_property: Node %u released to target '%s'", subject, value);
    }

    return 0;
}

void PipeWireManager::registry_event_global(void *data,
                                            uint32_t id,
                                            [[maybe_unused]] uint32_t permissions,
                                            const char *type,
                                            [[maybe_unused]] uint32_t version,
                                            const struct spa_dict *props) {
    PipeWireManager *manager = static_cast<PipeWireManager *>(data);

    // Get the default metadata object
    if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
        const char *metadata_name = spa_dict_lookup(props, PW_KEY_METADATA_NAME);
        if (metadata_name && strcmp(metadata_name, "default") == 0) {
            // Check if there already is a metadata object
            if (manager->metadata) {
                spa_hook_remove(&manager->metadata_listener);
                pw_proxy_destroy((pw_proxy *)manager->metadata);
                manager->metadata = nullptr;
            }

            // Save the metadata object for reuse later
            manager->metadata = static_cast<pw_metadata *>(
                pw_registry_bind(manager->registry, id, type, PW_VERSION_METADATA, 0));
            // Add listener for changes to node metadata
            spa_zero(manager->metadata_listener);
            pw_metadata_add_listener(manager->metadata, &manager->metadata_listener, &manager->metadata_events, manager);
        }
    }

    // Check if object is a node
    if (strcmp(type, PW_TYPE_INTERFACE_Node) != 0)
        return;

    // Get node name and media class
    const char *node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
    const char *node_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
    const char *node_media_role = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE);

    if (!node_media_class)
        return;

    // Ignore system notification sounds, they are very short and can cause crackling or even a crash
    if (std::ranges::find(ignored_node_names, node_name) != ignored_node_names.end())
        return;

    // Track output sinks to resolve stream targets to a device
    if (strcmp(node_media_class, "Audio/Sink") == 0 || strcmp(node_media_class, "Audio/Duplex") == 0) {
        if (node_name && strcmp(node_name, manager->capture_node_name) != 0) {
            const char *serial = spa_dict_lookup(props, PW_KEY_OBJECT_SERIAL);
            manager->sinks[id] = SinkInfo{node_name, serial ? serial : ""};
        }
        return;
    }

    // Check if object is an audio output stream, but exclude system events and our virtual surround manager source.
    if (strcmp(node_media_class, "Stream/Output/Audio") != 0)
        return;
    if (node_media_role && strcmp(node_media_role, "Notification") == 0)
        return;
    if (node_name && strcmp(node_name, manager->playback_node_name) == 0) {
        manager->playback_node_id = id;
        return;
    }

    if (manager->node_bindings.contains(id))
        return;

    // Bind the node to read target.object, which the registry global does not include
    pw_proxy *proxy = static_cast<pw_proxy *>(pw_registry_bind(manager->registry, id, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0));
    if (!proxy) {
        qWarning("registry_event_global: Failed to bind node with ID %u", id);
        return;
    }

    NodeBinding *binding = new NodeBinding{manager, id, proxy, {}};
    spa_zero(binding->listener);
    pw_node_add_listener(reinterpret_cast<pw_node *>(binding->proxy), &binding->listener, &manager->node_events, binding);
    manager->node_bindings.insert(id, binding);

    qDebug("registry_event_global: Tracking output stream '%s' with ID %u", node_name ? node_name : "(null)", id);
}

void PipeWireManager::registry_event_global_remove(void *data, uint32_t id) {
    PipeWireManager *manager = static_cast<PipeWireManager *>(data);

    manager->sinks.remove(id);
    manager->metadata_targets.remove(id);
    manager->routed_node_ids.remove(id);
    manager->decided_streams.remove(id);

    auto binding = manager->node_bindings.find(id);
    if (binding != manager->node_bindings.end()) {
        manager->destroy_binding(binding.value());
        manager->node_bindings.erase(binding);
    }
}

void PipeWireManager::destroy_binding(NodeBinding *binding) {
    spa_hook_remove(&binding->listener);
    pw_proxy_destroy(binding->proxy);
    delete binding;
}

bool PipeWireManager::create_virtual_surround_module(const string &hrir_wav_path, const string &channels) {
    if (module) {
        qInfo("create_virtual_surround_module: Virtual surround module already exists");
        return true;
    }
    if (hrir_wav_path.empty()) {
        qWarning("create_virtual_surround_module: No HRIR file provided");
        return false;
    }

    qInfo("create_virtual_surround_module: Creating module with %s channels and WAV path '%s'", channels.c_str(), hrir_wav_path.c_str());

    string args = "";

    // Set 7.1 filter graph for virtual surround module, automatically creating playback and capture nodes
    if (channels == "7.1") {
        const string filter_graph_7_1 = R"(
{
    nodes = [
        # duplicate inputs
        { type = builtin label = copy name = copyFL  }
        { type = builtin label = copy name = copyFR  }
        { type = builtin label = copy name = copyFC  }
        { type = builtin label = copy name = copyRL  }
        { type = builtin label = copy name = copyRR  }
        { type = builtin label = copy name = copySL  }
        { type = builtin label = copy name = copySR  }
        { type = builtin label = copy name = copyLFE }

        # apply hrir - HeSuVi 14-channel WAV (not the *-.wav variants) (note: 44 in HeSuVi are the same, but resampled to 44100)
        { type = builtin label = convolver name = convFL_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  0 } }
        { type = builtin label = convolver name = convFL_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  1 } }
        { type = builtin label = convolver name = convSL_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  2 } }
        { type = builtin label = convolver name = convSL_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  3 } }
        { type = builtin label = convolver name = convRL_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  4 } }
        { type = builtin label = convolver name = convRL_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  5 } }
        { type = builtin label = convolver name = convFC_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  6 } }
        { type = builtin label = convolver name = convFR_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  7 } }
        { type = builtin label = convolver name = convFR_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  8 } }
        { type = builtin label = convolver name = convSR_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  9 } }
        { type = builtin label = convolver name = convSR_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel = 10 } }
        { type = builtin label = convolver name = convRR_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel = 11 } }
        { type = builtin label = convolver name = convRR_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel = 12 } }
        { type = builtin label = convolver name = convFC_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel = 13 } }

        # treat LFE as FC
        { type = builtin label = convolver name = convLFE_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  6 } }
        { type = builtin label = convolver name = convLFE_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel = 13 } }

        # stereo output
        { type = builtin label = mixer name = mixL }
        { type = builtin label = mixer name = mixR }
    ]
    links = [
        # input
        { output = "copyFL:Out"  input="convFL_L:In"  }
        { output = "copyFL:Out"  input="convFL_R:In"  }
        { output = "copySL:Out"  input="convSL_L:In"  }
        { output = "copySL:Out"  input="convSL_R:In"  }
        { output = "copyRL:Out"  input="convRL_L:In"  }
        { output = "copyRL:Out"  input="convRL_R:In"  }
        { output = "copyFC:Out"  input="convFC_L:In"  }
        { output = "copyFR:Out"  input="convFR_R:In"  }
        { output = "copyFR:Out"  input="convFR_L:In"  }
        { output = "copySR:Out"  input="convSR_R:In"  }
        { output = "copySR:Out"  input="convSR_L:In"  }
        { output = "copyRR:Out"  input="convRR_R:In"  }
        { output = "copyRR:Out"  input="convRR_L:In"  }
        { output = "copyFC:Out"  input="convFC_R:In"  }
        { output = "copyLFE:Out" input="convLFE_L:In" }
        { output = "copyLFE:Out" input="convLFE_R:In" }

        # output
        { output = "convFL_L:Out"  input="mixL:In 1" }
        { output = "convFL_R:Out"  input="mixR:In 1" }
        { output = "convSL_L:Out"  input="mixL:In 2" }
        { output = "convSL_R:Out"  input="mixR:In 2" }
        { output = "convRL_L:Out"  input="mixL:In 3" }
        { output = "convRL_R:Out"  input="mixR:In 3" }
        { output = "convFC_L:Out"  input="mixL:In 4" }
        { output = "convFC_R:Out"  input="mixR:In 4" }
        { output = "convFR_R:Out"  input="mixR:In 5" }
        { output = "convFR_L:Out"  input="mixL:In 5" }
        { output = "convSR_R:Out"  input="mixR:In 6" }
        { output = "convSR_L:Out"  input="mixL:In 6" }
        { output = "convRR_R:Out"  input="mixR:In 7" }
        { output = "convRR_L:Out"  input="mixL:In 7" }
        { output = "convLFE_R:Out" input="mixR:In 8" }
        { output = "convLFE_L:Out" input="mixL:In 8" }
    ]
    inputs  = [ "copyFL:In" "copyFR:In" "copyFC:In" "copyLFE:In" "copyRL:In" "copyRR:In", "copySL:In", "copySR:In" ]
    outputs = [ "mixL:Out" "mixR:Out" ]
}
        )";

        // Set capture properties for virtual surround module
        const string capture_props_7_1 = R"(
{
    node.name = )" + (string)capture_node_name +
                                         R"(
    media.class = "Audio/Sink"
    audio.channels = 8
    audio.position = [ FL FR FC LFE RL RR SL SR ]
}
        )";

        // Set playback properties for virtual surround module
        const string playback_props_7_1 = R"(
{
    node.name = )" + string(playback_node_name) +
                                          R"(
    node.passive = true
    audio.channels = 2
    audio.position = [ FL FR ]
}
        )";

        // Combine the properites to a single JSON string
        args = R"(
{
    node.description = "Virtual Surround Manager",
    media.name = "Virtual Surround Manager",
    filter.graph = )" +
               filter_graph_7_1 +
               R"(,
    capture.props = )" +
               capture_props_7_1 + R"(,
    playback.props = )" +
               playback_props_7_1 + R"(
}
        )";
    }
    // Set 5.1 filter graph for virtual surround module, automatically creating playback and capture nodes
    else if (channels == "5.1") {
        const string filter_graph_5_1 = R"(
{
    nodes = [
        # duplicate inputs
        { type = builtin label = copy name = copyFL  }
        { type = builtin label = copy name = copyFR  }
        { type = builtin label = copy name = copySL  }
        { type = builtin label = copy name = copySR  }

        # apply hrir - HeSuVi 14-channel WAV (not the *-.wav variants) (note: 44 in HeSuVi are the same, but resampled to 44100)
        { type = builtin label = convolver name = convFL_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  0 } }
        { type = builtin label = convolver name = convFR_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  0 } }
        { type = builtin label = convolver name = convFL_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  1 } }
        { type = builtin label = convolver name = convFR_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  1 } }
        { type = builtin label = convolver name = convFC config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  2 } }
        { type = builtin label = convolver name = convLFE config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  3 } }
        { type = builtin label = convolver name = convSL_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  4 } }
        { type = builtin label = convolver name = convSR_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  4 } }
        { type = builtin label = convolver name = convSL_R config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  5 } }
        { type = builtin label = convolver name = convSR_L config = { filename = ")" +
                                        hrir_wav_path + R"(" channel =  5 } }

        # stereo output
        { type = builtin label = mixer name = mixL }
        { type = builtin label = mixer name = mixR }
    ]
    links = [
        # input
        { output = "copyFL:Out"  input="convFL_L:In"  }
        { output = "copyFL:Out"  input="convFL_R:In"  }
        { output = "copyFR:Out"  input="convFR_R:In"  }
        { output = "copyFR:Out"  input="convFR_L:In"  }
        { output = "copySL:Out"  input="convSL_L:In"  }
        { output = "copySL:Out"  input="convSL_R:In"  }
        { output = "copySR:Out"  input="convSR_R:In"  }
        { output = "copySR:Out"  input="convSR_L:In"  }

        # output
        { output = "convFL_L:Out"  input="mixL:In 1" }
        { output = "convFL_R:Out"  input="mixR:In 1" }
        { output = "convFR_L:Out"  input="mixL:In 2" }
        { output = "convFR_R:Out"  input="mixR:In 2" }
        { output = "convFC:Out"    input="mixL:In 3" }
        { output = "convFC:Out"    input="mixR:In 3" }
        { output = "convLFE:Out"   input="mixL:In 4" }
        { output = "convLFE:Out"   input="mixR:In 4" }
        { output = "convSL_L:Out"  input="mixL:In 5" }
        { output = "convSL_R:Out"  input="mixR:In 5" }
        { output = "convSR_L:Out"  input="mixL:In 6" }
        { output = "convSR_R:Out"  input="mixR:In 6" }
    ]
    inputs  = [ "copyFL:In" "copyFR:In" "convFC:In" "convLFE:In" "copySL:In" "copySR:In" ]
    outputs = [ "mixL:Out" "mixR:Out" ]
}
        )";

        // Set capture properties for virtual surround module
        const string capture_props_5_1 = R"(
{
    node.name = )" + (string)capture_node_name +
                                         R"(
    media.class = "Audio/Sink"
    audio.channels = 6
    audio.position = [ FL FR FC LFE SL SR ]
}
        )";

        // Set playback properties for virtual surround module
        const string playback_props_5_1 = R"(
{
    node.name = )" + string(playback_node_name) +
                                          R"(
    node.passive = true
    audio.channels = 2
    audio.position = [ FL FR ]
}
        )";

        // Combine the properites to a single JSON string
        args = R"(
{
    node.description = "Virtual Surround Manager",
    media.name = "Virtual Surround Manager",
    filter.graph = )" +
               filter_graph_5_1 +
               R"(,
    capture.props = )" +
               capture_props_5_1 + R"(,
    playback.props = )" +
               playback_props_5_1 + R"(
}
        )";
    } else {
        qWarning("create_virtual_surround_module: Invalid channels");
        Q_EMIT error_occured(QStringLiteral("Error creating virtual surround device. Invalid channels."));
        return false;
    }

    pw_thread_loop_lock(thread_loop);

    // Acutally load the module
    module = pw_context_load_module(
        context,
        "libpipewire-module-filter-chain",
        args.c_str(),
        nullptr);
    if (!module) {
        pw_thread_loop_unlock(thread_loop);
        qWarning("create_virtual_surround_module: Failed to load module into PipeWire context");
        Q_EMIT error_occured(QStringLiteral("Error creating virtual surround device."));
        return false;
    }

    pw_thread_loop_unlock(thread_loop);

    qInfo("create_virtual_surround_module: Successfully created module");

    return true;
}

void PipeWireManager::remove_virtual_surround_module() {
    pw_thread_loop_lock(thread_loop);

    if (module) {
        pw_impl_module_destroy(module);
        module = nullptr;
    }

    pw_thread_loop_unlock(thread_loop);

    qInfo("Removed virtual surround module");
}

void PipeWireManager::enable_routing() {
    // Do nothing if registry listener already exists
    if (is_registry_listener_added)
        return;

    pw_thread_loop_lock(thread_loop);

    if (registry)
        pw_proxy_destroy((pw_proxy *)registry);

    registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

    spa_zero(registry_listener);
    // Route all new audio output streams to the filter chain, only active while the loop runs
    pw_registry_add_listener(registry, &registry_listener, &registry_events, this);

    is_registry_listener_added = true;

    pw_thread_loop_unlock(thread_loop);

    qInfo("Enabled routing");
}

void PipeWireManager::disable_routing() {
    // Do nothing if registry listener does not exist
    if (!is_registry_listener_added)
        return;

    pw_thread_loop_lock(thread_loop);

    spa_hook_remove(&registry_listener);

    // Iterate over all nodes with our node name set as target.object, then disconnect them from the filter chain
    for (uint32_t id : routed_node_ids) {
        qDebug("disable_routing: clearing target.object for node %u", id);
        if (metadata) {
            // Remove the target object property from the node
            int ret = pw_metadata_set_property(metadata, id, PW_KEY_TARGET_OBJECT, "Spa:String:JSON", nullptr);
            qDebug("disable_routing: pw_metadata_set_property returned %d for node %u", ret, id);

            qDebug("Removed target.object property from node with ID %u", id);
        } else {
            qDebug("disable_routing: metadata is nullptr, skipping node %u", id);
        }
    }

    routed_node_ids.clear();

    for (auto it = node_bindings.begin(); it != node_bindings.end(); ++it)
        destroy_binding(it.value());
    node_bindings.clear();
    metadata_targets.clear();
    sinks.clear();
    decided_streams.clear();

    is_registry_listener_added = false;

    pw_thread_loop_unlock(thread_loop);

    qDebug("Disabled routing");
}

PipeWireManager::PipeWireManager() {
    // Initialize PipeWire library
    pw_init(nullptr, nullptr);

    // Debug: Output current version of PipeWire library
    qInfo("Compiled with libpipewire %s\n"
          "Linked with libpipewire %s\n",
          pw_get_headers_version(), pw_get_library_version());

    // Create some initial stuff for PipeWire to work
    thread_loop = pw_thread_loop_new("main", nullptr);
    if (!thread_loop) {
        qWarning("Failed to create PipeWire thread loop");
        Q_EMIT error_occured(QStringLiteral("Failed connecting to PipeWire audio service."));
        return;
    }

    pw_properties *props = pw_properties_new(nullptr, nullptr);
    pw_properties_set(props, PW_KEY_MEDIA_TYPE, "Audio");
    pw_properties_set(props, PW_KEY_MEDIA_ROLE, "Music");

    // Required for Flatpak, otherwise it cannot write PipeWire metadata! Also nice to have for native packaging
    pw_properties_set(props, PW_KEY_MEDIA_CATEGORY, "Manager");

    context = pw_context_new(pw_thread_loop_get_loop(thread_loop), props, 0);
    if (!context) {
        pw_thread_loop_unlock(thread_loop);
        qWarning("Failed to create PipeWire context");
        Q_EMIT error_occured(QStringLiteral("Failed connecting to PipeWire audio service."));
        return;
    }

    // Always lock before touching loop objects from outside the thread, then unlock!
    pw_thread_loop_lock(thread_loop);

    core = pw_context_connect(context, nullptr, 0);
    if (!core) {
        pw_thread_loop_unlock(thread_loop);
        qWarning("Failed to connect to PipeWire daemon");
        Q_EMIT error_occured(QStringLiteral("Failed connecting to PipeWire audio service."));
        return;
    }

    registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
    if (!registry) {
        pw_thread_loop_unlock(thread_loop);
        qWarning("Failed to get PipeWire registry");
        Q_EMIT error_occured(QStringLiteral("Failed connecting to PipeWire audio service."));
        return;
    }

    if (pw_thread_loop_start(thread_loop) < 0) {
        pw_thread_loop_unlock(thread_loop);
        qWarning("Failed to start the thread loop.");
        Q_EMIT error_occured(QStringLiteral("Failed connecting to PipeWire audio service."));
        return;
    }

    pw_thread_loop_unlock(thread_loop);
}

PipeWireManager::~PipeWireManager() {
    disable_routing();
    // We need to wait a little bit or the audio just stops completely when destroying too fast
    QThread::msleep(100);

    // Clean up PipeWire stuff
    pw_thread_loop_stop(thread_loop);
    pw_core_disconnect(core);
    pw_context_destroy(context);
    pw_thread_loop_destroy(thread_loop);
    pw_deinit();
}
