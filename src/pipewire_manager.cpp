#include "pipewire_manager.h"
#include <qdebug.h>
#include <qhashfunctions.h>
#include <qlogging.h>
#include <qtmetamacros.h>

int PipeWireManager::on_metadata_property([[maybe_unused]] void *data,
                                          uint32_t subject, // Node ID
                                          const char *key,
                                          [[maybe_unused]] const char *type,
                                          const char *value) { // NULL if property removed
    PipeWireManager *manager = static_cast<PipeWireManager *>(data);

    if (!key)
        return 0;

    // Check if the target.object property changes, but exclude our virtual surround manager source
    if (strcmp(key, PW_KEY_TARGET_OBJECT) != 0 && strcmp(key, "target.node") != 0)
        return 0;
    if (value != NULL) {
        return 0;
    }
    if (subject == manager->playback_node_id)
        return 0;

    pw_metadata_set_property(manager->metadata, subject, PW_KEY_TARGET_OBJECT, "Spa:String:JSON", manager->capture_node_name);

    //  Debug: Print info message
    qInfo("on_metadata_property: Node with ID %u routed to sink '%s'.\n", subject, manager->capture_node_name);

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
            // Save the metadata object for reuse later
            manager->metadata = static_cast<pw_metadata *>(pw_registry_bind(manager->registry, id, type, PW_VERSION_METADATA, 0));
            // Add listener for changes to node metadata
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

    // Debug: Print object details to console.
    // printf("object: id: %u | name: %s | type: %s/%d\n", id, node_name, type, version);

    // Check if object is an audio output stream, but exclude system events and our virtual surround manager source.
    if (!node_media_class || strcmp(node_media_class, "Stream/Output/Audio") != 0)
        return;
    if (node_media_role && strcmp(node_media_role, "Notification") == 0)
        return;
    if (node_name && strcmp(node_name, manager->playback_node_name) == 0) {
        manager->playback_node_id = id;
        return;
    }

    if (manager->metadata) {
        // Route the audio output node to our virtual surround manager sink
        pw_metadata_set_property(manager->metadata, id, PW_KEY_TARGET_OBJECT, "Spa:String:JSON", manager->capture_node_name);

        // printf("test4");
        //  Debug: Print info message
        printf("registry_event_global: Node '%s' with ID %u routed to sink '%s'.\n", node_name, id, manager->capture_node_name);
    }
}

void PipeWireManager::create_virtual_surround_node() {
    // Set filter graph for virtual surround module, automatically creating playback and capture nodes
    const string filter_graph = R"(
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

        # apply hrir - HeSuVi 14-channel WAV (not the *-.wav variants) (note: */44/* in HeSuVi are the same, but resampled to 44100)
        { type = builtin label = convolver name = convFL_L config = { filename = )" +
                                hrir_wav_path + R"( channel =  0 } }
        { type = builtin label = convolver name = convFL_R config = { filename = )" +
                                hrir_wav_path + R"( channel =  1 } }
        { type = builtin label = convolver name = convSL_L config = { filename = )" +
                                hrir_wav_path + R"( channel =  2 } }
        { type = builtin label = convolver name = convSL_R config = { filename = )" +
                                hrir_wav_path + R"( channel =  3 } }
        { type = builtin label = convolver name = convRL_L config = { filename = )" +
                                hrir_wav_path + R"( channel =  4 } }
        { type = builtin label = convolver name = convRL_R config = { filename = )" +
                                hrir_wav_path + R"( channel =  5 } }
        { type = builtin label = convolver name = convFC_L config = { filename = )" +
                                hrir_wav_path + R"( channel =  6 } }
        { type = builtin label = convolver name = convFR_R config = { filename = )" +
                                hrir_wav_path + R"( channel =  7 } }
        { type = builtin label = convolver name = convFR_L config = { filename = )" +
                                hrir_wav_path + R"( channel =  8 } }
        { type = builtin label = convolver name = convSR_R config = { filename = )" +
                                hrir_wav_path + R"( channel =  9 } }
        { type = builtin label = convolver name = convSR_L config = { filename = )" +
                                hrir_wav_path + R"( channel = 10 } }
        { type = builtin label = convolver name = convRR_R config = { filename = )" +
                                hrir_wav_path + R"( channel = 11 } }
        { type = builtin label = convolver name = convRR_L config = { filename = )" +
                                hrir_wav_path + R"( channel = 12 } }
        { type = builtin label = convolver name = convFC_R config = { filename = )" +
                                hrir_wav_path + R"( channel = 13 } }

        # treat LFE as FC
        { type = builtin label = convolver name = convLFE_L config = { filename = )" +
                                hrir_wav_path + R"( channel =  6 } }
        { type = builtin label = convolver name = convLFE_R config = { filename = )" +
                                hrir_wav_path + R"( channel = 13 } }

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
    const string capture_props = R"(
{
    node.name = )" + string(capture_node_name) +
                                 R"(
    media.class = "Audio/Sink"
    audio.channels = 8
    audio.position = [ FL FR FC LFE RL RR SL SR ]
}
    )";

    // Set playback properties for virtual surround module
    const string playback_props = R"(
{
    node.name = )" + string(playback_node_name) +
                                  R"(
    node.passive = true
    audio.channels = 2
    audio.position = [ FL FR ]
}
    )";

    // Combine the properites to a single JSON string
    const string args = R"(
{
    node.description = "Virtual Surround Manager",
    media.name = "Virtual Surround Manager",
    filter.graph = )" + filter_graph +
                        R"(,
    capture.props = )" +
                        capture_props + R"(,
    playback.props = )" +
                        playback_props + R"(
}
    )";

    // Acutally load the module
    pw_context_load_module(
        context,
        "libpipewire-module-filter-chain",
        args.c_str(),
        NULL);
}

PipeWireManager::PipeWireManager() {
    // Initialize PipeWire library
    pw_init(NULL, NULL);

    // Debug: Output current version of PipeWire library
    qInfo("Compiled with libpipewire %s\n"
          "Linked with libpipewire %s\n",
          pw_get_headers_version(), pw_get_library_version());

    // Create some initial stuff for PipeWire to work
    loop = pw_main_loop_new(NULL);
    if (!loop) {
        qFatal("Failed to create PipeWire main loop\n");
        Q_EMIT errorOccured(QStringLiteral("Error connecting to PipeWire audio service."));
        return;
    }
    context = pw_context_new(pw_main_loop_get_loop(loop), NULL, 0);
    if (!context) {
        qFatal("Failed to create PipeWire context\n");
        Q_EMIT errorOccured(QStringLiteral("Error connecting to PipeWire audio service."));
        return;
    }
    core = pw_context_connect(context, NULL, 0);
    if (!core) {
        qFatal("Failed to connect to PipeWire daemon\n");
        Q_EMIT errorOccured(QStringLiteral("Error connecting to PipeWire audio service."));
        return;
    }
    registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
    if (!registry) {
        qFatal("Failed to get PipeWire registry\n");
        Q_EMIT errorOccured(QStringLiteral("Error connecting to PipeWire audio service."));
        return;
    }
    spa_zero(registry_listener);
    spa_zero(metadata_listener);

    create_virtual_surround_node();

    // Route all new audio output streams to the filter chain
    pw_registry_add_listener(registry, &registry_listener, &registry_events, this);

    virtualSurroundEnabled = true;

    // Run the main PipeWire loop
    pw_main_loop_run(loop);
}

PipeWireManager::~PipeWireManager() {
    // Clean up PipeWire stuff
    pw_core_disconnect(core);
    pw_context_destroy(context);
    pw_main_loop_destroy(loop);
    pw_deinit();

    virtualSurroundEnabled = false;
}