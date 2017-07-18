#pragma once

/*
 * D-Bus XML Config Parser
 */

#include <c-list.h>
#include <c-macro.h>
#include <expat.h>
#include <stdlib.h>

typedef struct ConfigPath ConfigPath;
typedef struct ConfigNode ConfigNode;
typedef struct ConfigParser ConfigParser;
typedef struct ConfigRoot ConfigRoot;
typedef struct ConfigState ConfigState;

#define CONFIG_PARSER_BUFFER_MAX 4096

enum {
        _CONFIG_E_SUCCESS,

        CONFIG_E_INVALID,
};

enum {
        CONFIG_NODE_NONE,

        CONFIG_NODE_BUSCONFIG,
        CONFIG_NODE_USER,
        CONFIG_NODE_TYPE,
        CONFIG_NODE_FORK,
        CONFIG_NODE_SYSLOG,
        CONFIG_NODE_KEEP_UMASK,
        CONFIG_NODE_LISTEN,
        CONFIG_NODE_PIDFILE,
        CONFIG_NODE_INCLUDEDIR,
        CONFIG_NODE_STANDARD_SESSION_SERVICEDIRS,
        CONFIG_NODE_STANDARD_SYSTEM_SERVICEDIRS,
        CONFIG_NODE_SERVICEDIR,
        CONFIG_NODE_SERVICEHELPER,
        CONFIG_NODE_AUTH,
        CONFIG_NODE_INCLUDE,
        CONFIG_NODE_POLICY,
        CONFIG_NODE_LIMIT,
        CONFIG_NODE_SELINUX,
        CONFIG_NODE_APPARMOR,
        CONFIG_NODE_ALLOW,
        CONFIG_NODE_DENY,
        CONFIG_NODE_ASSOCIATE,

        _CONFIG_NODE_N,
};

enum {
        CONFIG_APPARMOR_ENABLED,
        CONFIG_APPARMOR_DISABLED,
        CONFIG_APPARMOR_REQUIRED,
};

struct ConfigPath {
        unsigned long n_refs;
        ConfigPath *parent;
        bool is_dir;
        char path[];
};

#define CONFIG_FILE_NULL(_x) {                                                  \
                .n_refs = 1,                                                    \
        }

struct ConfigNode {
        CList root_link;
        CList include_link;
        ConfigNode *parent;
        size_t n_children;

        char *cdata;
        size_t n_cdata;

        unsigned int type;

        union {
                struct {
                        ConfigPath *dir;
                } includedir;

                struct {
                        ConfigPath *file;
                        bool ignore_missing : 1;
                        bool if_selinux_enabled : 1;
                        bool selinux_root_relative : 1;
                } include;

                struct {
                        char *user;
                        char *group;
                        bool mandatory : 1;
                        bool at_console : 1;
                } policy;

                struct {
                        char *name;
                } limit;

                struct {
                        unsigned int mode;
                } apparmor;

                struct {
                        char *send_interface;
                        char *send_member;
                        char *send_error;
                        char *send_destination;
                        char *send_path;
                        char *send_type;
                        char *recv_interface;
                        char *recv_member;
                        char *recv_error;
                        char *recv_sender;
                        char *recv_path;
                        char *recv_type;
                        char *own;
                        char *own_prefix;
                        char *user;
                        char *group;
                        bool send_requested_reply : 1;
                        bool recv_requested_reply : 1;
                        bool eavesdrop : 1;
                        bool log : 1;
                } allow_deny;

                struct {
                        char *own;
                        char *context;
                } associate;
        };
};

#define CONFIG_NODE_NULL(_x) {                                                  \
                .root_link = C_LIST_INIT((_x).root_link),                       \
                .include_link = C_LIST_INIT((_x).include_link),                 \
                .type = CONFIG_NODE_NONE,                                       \
        }

struct ConfigRoot {
        CList node_list;
        CList include_list;
};

#define CONFIG_ROOT_NULL(_x) {                                                  \
                .node_list = C_LIST_INIT((_x).node_list),                       \
                .include_list = C_LIST_INIT((_x).include_list),                 \
        }

struct ConfigParser {
        XML_Parser xml;

        struct ConfigState {
                ConfigPath *file;
                ConfigRoot *root;
                ConfigNode *current;
                ConfigNode *last;
                size_t n_depth;
                size_t n_failed;
                int error;
        } state;
};

#define CONFIG_PARSER_NULL(_x) {                                                \
                .xml = NULL,                                                    \
        }

/* files */

int config_path_new(ConfigPath **filep, ConfigPath *parent, const char *path);
ConfigPath *config_path_ref(ConfigPath *file);
ConfigPath *config_path_unref(ConfigPath *file);

C_DEFINE_CLEANUP(ConfigPath *, config_path_unref);

/* nodes */

int config_node_new(ConfigNode **nodep, ConfigNode *parent, unsigned int type);
ConfigNode *config_node_free(ConfigNode *node);

C_DEFINE_CLEANUP(ConfigNode *, config_node_free);

/* roots */

int config_root_new(ConfigRoot **rootp);
ConfigRoot *config_root_free(ConfigRoot *root);

C_DEFINE_CLEANUP(ConfigRoot *, config_root_free);

/* parser */

void config_parser_init(ConfigParser *parser);
void config_parser_deinit(ConfigParser *parser);

int config_parser_read(ConfigParser *parser, ConfigRoot **rootp, const char *path);

C_DEFINE_CLEANUP(ConfigParser *, config_parser_deinit);