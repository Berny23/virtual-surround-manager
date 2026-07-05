#pragma once

#include "pipewire/context.h"
#include "pipewire/impl-module.h"
#include "pipewire/thread-loop.h"
#include <QDebug>
#include <QMap>
#include <QObject>
#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/filter.h>
#include <pipewire/impl-module.h>
#include <pipewire/impl-node.h>
#include <pipewire/keys.h>
#include <pipewire/main-loop.h>
#include <pipewire/node.h>
#include <pipewire/pipewire.h>
#include <pipewire/properties.h>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qhashfunctions.h>
#include <qlogging.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <string>

using std::string;

class PipeWireManager : public QObject {
    Q_OBJECT

  public:
    PipeWireManager();
    ~PipeWireManager();

    //
    // Creates a new virtual surround module. Does nothing if a module already exists.
    // Call this before enable_routing().
    //
    bool create_virtual_surround_module(const string &hrir_wav_path);

    //
    // Removes the virtual surround module.
    //
    void remove_virtual_surround_module();

    //
    // Starts the PipeWire loop, enabling routing of audio data through the virtual surround node.
    //
    void enable_routing();

    //
    // Stops the PipeWire loop, disabling routing of audio data through the virtual surround node.
    //
    void disable_routing();

    Q_SIGNAL void error_occured(const QString &message);

  private:
    struct pw_thread_loop *thread_loop = nullptr;
    struct pw_context *context = nullptr;
    struct pw_core *core = nullptr;
    struct pw_registry *registry = nullptr;
    struct pw_metadata *metadata = nullptr;
    struct spa_hook registry_listener;
    struct spa_hook metadata_listener;
    struct spa_hook core_listener;

    const char *capture_node_name = "effect_input.virtual-surround-manager";
    const char *playback_node_name = "effect_output.virtual-surround-manager";

    bool is_registry_listener_added = false;
    QSet<uint32_t> routed_node_ids;
    QSet<uint32_t> decided_streams;
    uint32_t playback_node_id = SPA_ID_INVALID;
    pw_impl_module *module = nullptr;

    string default_sink_name;

    QMap<uint32_t, string> metadata_targets;

    struct SinkInfo {
        string node_name;
        string serial;
    };
    QMap<uint32_t, SinkInfo> sinks;

    struct NodeBinding {
        PipeWireManager *manager;
        uint32_t id;
        struct pw_proxy *proxy;
        struct spa_hook listener;
    };
    QMap<uint32_t, NodeBinding *> node_bindings;

    //
    // Removes a node binding's listener and destroys its proxy.
    //
    void destroy_binding(NodeBinding *binding);

    //
    // Resolves a target (node name or object serial) to a known sink's node name.
    //
    string resolve_sink_name(const string &key) const;

    //
    // Returns true if a stream has no explicit target or targets the default sink.
    //
    bool stream_follows_default(uint32_t id, const string &props_target) const;

    //
    // Decides once whether to virtualize a stream, then routes it if so.
    // Deciding only once avoids fighting with apps that toggle their own target repeatedly.
    //
    void decide_stream(uint32_t id, const string &props_target);

    //
    // Called when a metadata property of a node changes.
    // This is required for compatibility with EasyEffects etc.
    //
    static int on_metadata_property(void *data,
                                    uint32_t subject, // Node ID
                                    const char *key,
                                    [[maybe_unused]] const char *type,
                                    const char *value);

    const struct pw_metadata_events metadata_events = {
        .version = PW_VERSION_METADATA_EVENTS,
        .property = on_metadata_property,
    };

    //
    // Called when a new object (node) is created in the PipeWire registry.
    //
    static void registry_event_global(void *data,
                                      uint32_t id,
                                      [[maybe_unused]] uint32_t permissions,
                                      const char *type,
                                      [[maybe_unused]] uint32_t version,
                                      const struct spa_dict *props);

    static void registry_event_global_remove(void *data, uint32_t id);

    const struct pw_registry_events registry_events = {
        .version = PW_VERSION_REGISTRY_EVENTS,
        .global = registry_event_global,
        .global_remove = registry_event_global_remove,
    };

    //
    // Called with a bound output node's info; reads target.object from its full props.
    //
    static void on_node_info(void *data, const struct pw_node_info *info);

    const struct pw_node_events node_events = {
        .version = PW_VERSION_NODE_EVENTS,
        .info = on_node_info,
        .param = nullptr,
    };
};
