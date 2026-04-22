#pragma once

#include "pipewire/context.h"
#include "pipewire/impl-module.h"
#include "pipewire/thread-loop.h"
#include <QDebug>
#include <QObject>
#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/filter.h>
#include <pipewire/impl-module.h>
#include <pipewire/impl-node.h>
#include <pipewire/keys.h>
#include <pipewire/main-loop.h>
#include <pipewire/pipewire.h>
#include <pipewire/properties.h>
#include <qdebug.h>
#include <qhashfunctions.h>
#include <qlogging.h>
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
    void create_virtual_surround_module(const string &hrir_wav_path);

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
    struct pw_thread_loop *thread_loop;
    struct pw_context *context;
    struct pw_core *core;
    struct pw_registry *registry;
    struct pw_metadata *metadata;
    struct spa_hook registry_listener;
    struct spa_hook metadata_listener;
    struct spa_hook core_listener;

    const char *capture_node_name = "effect_input.virtual-surround-manager";
    const char *playback_node_name = "effect_output.virtual-surround-manager";

    // TODO: Changeable in UI
    // string hrir_wav_path = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav";

    bool is_registry_listener_added = false;
    QSet<uint32_t> routed_node_ids;
    uint32_t playback_node_id;
    pw_impl_module *module = nullptr;

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

    const struct pw_registry_events registry_events = {
        .version = PW_VERSION_REGISTRY_EVENTS,
        .global = registry_event_global,
        .global_remove = nullptr,
    };
};