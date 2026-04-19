#pragma once
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
#include <qtmetamacros.h>
#include <string>

using std::string;

class PipeWireManager : public QObject {
    Q_OBJECT

  public:
    PipeWireManager();
    ~PipeWireManager();

  Q_SIGNALS:
    void errorOccured(const QString &message);

  private:
    struct pw_main_loop *loop;
    struct pw_context *context;
    struct pw_core *core;
    struct pw_registry *registry;
    struct spa_hook registry_listener;
    struct spa_hook metadata_listener;
    struct pw_metadata *metadata;

    const char *capture_node_name = "effect_input.virtual-surround-manager";
    const char *playback_node_name = "effect_output.virtual-surround-manager";

    // TODO: Changeable in UI
    string hrir_wav_path = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav";

    bool virtualSurroundEnabled = true; // TODO: Remove test true

    uint32_t playback_node_id;

    //
    // Called when a metadata property of a node changes.
    // This is required for compatibility with EasyEffects etc.
    //
    static int on_metadata_property([[maybe_unused]] void *data,
                                    uint32_t subject, // Node ID
                                    const char *key,
                                    [[maybe_unused]] const char *type,
                                    const char *value);

    const struct pw_metadata_events metadata_events = {
        PW_VERSION_METADATA_EVENTS,
        on_metadata_property,
    };

    //
    // Called when a new object (node) is created in the PipeWire registry.
    //
    static void registry_event_global([[maybe_unused]] void *data,
                                      uint32_t id,
                                      [[maybe_unused]] uint32_t permissions,
                                      const char *type,
                                      [[maybe_unused]] uint32_t version,
                                      const struct spa_dict *props);

    const struct pw_registry_events registry_events = {
        PW_VERSION_REGISTRY_EVENTS,
        registry_event_global,
        NULL,
    };

    void create_virtual_surround_node();
};