#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define VERSION "1.0.1"

using std::cout;
using std::cerr;
using std::endl;

struct CLIArg {
    std::vector<std::string> filenames;
    enum class Mode {
        x = 0,
        o,
        c,
        d
    } mode;
};

int analyz_arg(int argc, char** argv, CLIArg* cliargp);
int analyz_opt(std::string arg, CLIArg* cliargp);
int analyz_opt_long(std::string arg, CLIArg* cliargp);

int dump(std::string filename, CLIArg::Mode mode);

std::string byte_to_str(char byte, CLIArg::Mode mode);
int is_escapable(char byte);
void sprintesc(char* ptr, char byte);

std::string mode_to_str(CLIArg::Mode mode);

void help();
void version();

int main(int argc, char** argv) {
    int i, ret = 0;
    CLIArg cliarg;

    ret = analyz_arg(argc, argv, &cliarg);
    if (ret)
        return ret;

    for (i = 0; i < cliarg.filenames.size(); i++) {
        ret = dump(cliarg.filenames[i], cliarg.mode);
        if (ret)
            return ret;
    }

    return ret;
}

int analyz_arg(int argc, char** argv, CLIArg* cliargp) {
    int i, ret = 0;
    int opt_flag = 1;
    std::string arg;

    *cliargp = {};

    for (i = 1; i < argc; i++) {
        arg = argv[i];

        if (arg == "--") {
            opt_flag = 0;
        } else if (opt_flag && arg[0] == '-') {
            ret = analyz_opt(arg, cliargp);
            if (ret)
                return ret;
        } else {
            cliargp->filenames.push_back(arg);
        }
    }

    if (cliargp->filenames.empty()) {
        cerr << "dumper: No file specified." << endl;
        ret = 1;
    }

    return ret;
}

int analyz_opt(std::string arg, CLIArg* cliargp) {
    int i;

    if (arg[1] == '-')
        return analyz_opt_long(arg, cliargp);

    if (arg.length() == 1) {
        cerr << "dumper: Unknown option: -" << endl;
        return 1;
    }

    for (i = 1; arg[i]; i++) {
        switch (arg[i]) {
        case 'x':
            cliargp->mode = CLIArg::Mode::x;
            break;
        case 'o':
            cliargp->mode = CLIArg::Mode::o;
            break;
        case 'c':
            cliargp->mode = CLIArg::Mode::c;
            break;
        case 'd':
            cliargp->mode = CLIArg::Mode::d;
            break;
        case 'h':
            help();
            std::exit(0);
        case 'v':
            version();
            std::exit(0);
        default:
            cerr << "dumper: Unknown option: -" << arg[i] << endl;
            return 1;
        }
    }

    return 0;
}

int analyz_opt_long(std::string arg, CLIArg* cliargp) {
    if (arg == "--hex") {
        cliargp->mode = CLIArg::Mode::x;
    } else if (arg == "--octal") {
        cliargp->mode = CLIArg::Mode::o;
    } else if (arg == "--char") {
        cliargp->mode = CLIArg::Mode::c;
    } else if (arg == "--dec") {
        cliargp->mode = CLIArg::Mode::d;
    } else if (arg == "--help") {
        help();
        std::exit(0);
    } else if (arg == "--version") {
        version();
        std::exit(0);
    } else {
        cerr << "dumper: Unknown option: " << arg << endl;
        return 1;
    }

    return 0;
}

int dump(std::string filename, CLIArg::Mode mode) {
    int i = 0;
    char byte;
    std::string byte_str;
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs) {
        cerr << "dumper: Failed to open " << filename << endl;
        return 1;
    }

    while (ifs.read(&byte, 1)) {
        if (i > 15) {
            i = 0;
            cout << endl;
        }
        byte_str = byte_to_str(byte, mode);
        cout << byte_str << " ";
        i++;
    }

    if (i < 16)
        cout << endl;

    ifs.close();

    return 0;
}

std::string byte_to_str(char byte, CLIArg::Mode mode) {
    char output[16];
    std::string ret;

    switch (mode) {
    case CLIArg::Mode::x:
        std::sprintf(output, "%02x", byte);
        break;
    case CLIArg::Mode::o:
        std::sprintf(output, "%03o", byte);
        break;
    case CLIArg::Mode::c:
        if ('!' < byte && byte < '~')
            std::sprintf(output, " %c", byte);
        else if (is_escapable(byte))
            sprintesc(output, byte);
        else
            std::sprintf(output, "%02x", byte);
        break;
    case CLIArg::Mode::d:
        std::sprintf(output, "%03d", byte);
        break;
    default:
        return "";
    }

    ret = output;

    return ret;
}

int is_escapable(char byte) {
    switch (byte) {
    case '\a': case '\b':
    case '\033': case '\f':
    case '\n': case '\r':
    case '\t': case '\v':
        return 1;
    default:
        return 0;
    }
}

void sprintesc(char* ptr, char byte) {
    std::string ret;
    switch (byte) {
    case '\a':
        ret = "\\a";
        break;
    case '\b':
        ret = "\\b";
        break;
    case '\033':
        ret = "\\e";
        break;
    case '\f':
        ret = "\\f";
        break;
    case '\n':
        ret = "\\n";
        break;
    case '\r':
        ret = "\\r";
        break;
    case '\t':
        ret = "\\t";
        break;
    case '\v':
        ret = "\\v";
        break;
    default:
        std::abort();
    }

    std::strcpy(ptr, ret.c_str());
}

std::string mode_to_str(CLIArg::Mode mode) {
    switch (mode) {
    case CLIArg::Mode::x:
        return "hex";
    case CLIArg::Mode::o:
        return "octal";
    case CLIArg::Mode::c:
        return "char";
    case CLIArg::Mode::d:
        return "dec";
    default:
        return "";
    }
}

void help() {
    std::printf(
        "usage: dumper [option] <filename>...\n"
        "Display file content per byte.\n"
        "\n"
        "options:\n"
        "   -x, --hex      One byte hex (default)\n"
        "   -o, --octal    One byte octal\n"
        "   -c, --char     One byte char (Non printable are in hex)\n"
        "   -d, --dec      One byte decimal\n"
        "\n"
        "   -h, --help     Print this help\n"
        "   -v, --version  Print the version of dumper\n"
    );
}

void version() {
    std::printf(
        "dumper  %s\n"
        "\n"
        "Written by Yutaka Goy\n"
        "in C++, on 2026-04-25\n"
        , VERSION);
}
