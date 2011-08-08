#include <sys/resource.h>
#include <google/protobuf/stubs/common.h>   // for `ShutdownProtobufLibrary()`

#include "stats/control.hpp"
#include "server/server.hpp"
#include "fsck/fsck.hpp"
#include "extract/extract.hpp"
#include "utils.hpp"
#include "help.hpp"
#include "migrate/migrate.hpp"
#include "http/http.hpp"

void print_version_message() {
    printf("rethinkdb " RETHINKDB_VERSION
#ifndef NDEBUG
           " (debug)"
#endif
           "\n");
}

void usage() {
    Help_Pager *help = Help_Pager::instance();
    help->pagef("Usage: rethinkdb COMMAND ...\n"
                "Commands:\n"
                "    help        Display help about rethinkdb and rethinkdb commands.\n"
                "\n"
                "Creating and serving databases:\n"
                "    create      Create an empty database.\n"
                "    serve       Serve an existing database.\n"
                "\n"
                "Administrating databases:\n"
                "    extract     Extract as much data as possible from a corrupted database.\n"
                "    import      Import data from raw memcached commands.\n"
                "    fsck        Check a database for corruption.\n"
                "    migrate     Convert between file versions.\n"
                "\n"
                "Use 'rethinkdb help COMMAND' for help on a single command.\n"
                "Use 'rethinkdb --version' for the current version of rethinkdb.\n");
}

int dispatch_on_args(std::vector<char *> args);

int main(int argc, char *argv[]) {
    initialize_precise_time();
    install_generic_crash_handler();

    atexit(&google::protobuf::ShutdownProtobufLibrary);

#ifndef NDEBUG
    rlimit core_limit;
    core_limit.rlim_cur = RLIM_INFINITY;
    core_limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &core_limit);
#endif

    /* Put arguments into a vector so we can modify them if convenient */
    std::vector<char *> args(argv, argv + argc);

    return dispatch_on_args(args);
}

int dispatch_on_args(std::vector<char *> args) {
    /* Default to "rethinkdb serve" */
    if (args.size() == 1) args.push_back(const_cast<char *>("serve"));

    /* Switch based on subcommand, then dispatch to the appropriate function */
    if (!strcmp(args[1], "serve") || !strcmp(args[1], "create") || !strcmp(args[1], "import")) {
        return run_server(args.size() - 1, args.data() + 1);

    } else if (!strcmp(args[1], "extract")) {
        return run_extract(args.size() - 1, args.data() + 1);

    } else if (!strcmp(args[1], "fsck")) {
        return run_fsck(args.size() - 1, args.data() + 1);

    } else if (!strcmp(args[1], "migrate")) {
        return run_migrate(args.size(), args.data()); //migrate requires the exec path

    } else if (!strcmp(args[1], "help") || !strcmp(args[1], "-h") || !strcmp(args[1], "--help")) {
        if (args.size() >= 3) {
            if (!strcmp(args[2], "serve")) {
                usage_serve();
            } else if (!strcmp(args[2], "create")) {
                usage_create();
            } else if (!strcmp(args[2], "import")) {
                usage_import();
            } else if (!strcmp(args[2], "extract")) {
                extract::usage(args[1]);
            } else if (!strcmp(args[2], "fsck")) {
                fsck::usage(args[1]);
            } else if (!strcmp(args[2], "migrate")) {
                migrate::usage(args[1]);
            } else {
                printf("No such command %s.\n", args[2]);
            }
        } else {
            usage();
        }

    } else if (!strcmp(args[1], "--version")) {
        print_version_message();
    } else {
        /* Default to "rethinkdb serve"; we get here if we are run without an explicit subcommand
        but with at least one flag */
        args.insert(args.begin() + 1, const_cast<char *>("serve"));
        return run_server(args.size() - 1, args.data() + 1);
    }

    return 0;
}
