/*
 * Copyright (c) 2015 Andrew Kelley
 *
 * This file is part of cdoc, which is MIT licensed.
 * See http://opensource.org/licenses/MIT
 */

#include "config.h"
#include <clang-c/Index.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

__attribute__ ((cold))
__attribute__ ((noreturn))
__attribute__ ((format (printf, 1, 2)))
static void panic(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    abort();
}

static int usage(char *arg0) {
    fprintf(stderr, "Usage: %s target [-- clang-args]\n", arg0);
    return 1;
}

struct CDoc {
    CXTranslationUnit tu;
};

static enum CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    //struct CDoc *cdoc = (struct CDoc *)client_data;
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation location = clang_getRangeStart(range);
    enum CXCursorKind kind = clang_getCursorKind(cursor);
    CXString name = clang_getCursorSpelling(cursor);

    switch (kind) {
    case CXCursor_StructDecl:
        {
            fprintf(stderr, "struct %s\n", clang_getCString(name));
            break;
        }
    case CXCursor_EnumDecl:
        {
            fprintf(stderr, "enum %s\n", clang_getCString(name));
            break;
        }
    case CXCursor_EnumConstantDecl:
        {
            fprintf(stderr, "enum value %s\n", clang_getCString(name));
            break;
        }
    case CXCursor_FieldDecl:
        {
            fprintf(stderr, "field %s\n", clang_getCString(name));
            return CXChildVisit_Continue;
        }
    case CXCursor_FunctionDecl:
        {
            fprintf(stderr, "function %s\n", clang_getCString(name));
            break;
        }
    case CXCursor_TypeRef:
        {
            fprintf(stderr, "typeref %s\n", clang_getCString(name));
            break;
        }
    case CXCursor_ParmDecl:
        {
            fprintf(stderr, "param %s\n", clang_getCString(name));
            break;
        }
    case CXCursor_UnexposedAttr:
        return CXChildVisit_Continue;
    case CXCursor_CompoundStmt:
        return CXChildVisit_Continue;
    default:
        {
            CXFile file;
            unsigned line, column, offset;
            clang_getFileLocation(location, &file, &line, &column, &offset);
            CXString file_name = clang_getFileName(file);

            panic("%s line %u, column %u: Unrecognized token: %s %d %d\n",
                    clang_getCString(file_name), line, column,
                    clang_getCString(name), (int)kind, (int)CXCursor_BreakStmt);
        }
    }
    return CXChildVisit_Recurse;
}

#define MAX_TARGETS 100
static char *targets[MAX_TARGETS];

int main(int argc, char **argv) {
    char *arg0 = argv[0];

    struct CDoc cdoc;

    char **compile_args = NULL;
    int compile_args_len = 0;

    int target_count = 0;

    for (int i = 1; i < argc; i += 1) {
        char *arg = argv[i];
        if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
            compile_args = &argv[i + 1];
            compile_args_len = argc - (i + 1);
            break;
        } else if (target_count < MAX_TARGETS) {
            targets[target_count++] = argv[i];
        } else {
            return usage(arg0);
        }
    }

    CXIndex index = clang_createIndex(1, 0);

    for (int i = 0; i < target_count; i += 1) {
        char *target = targets[i];

        enum CXErrorCode err_code;
        if ((err_code = clang_parseTranslationUnit2(index, target,
                (const char **)compile_args, compile_args_len,
                NULL, 0, CXTranslationUnit_None, &cdoc.tu)))
        {
            panic("parse translation unit failure");
        }

        unsigned diag_count = clang_getNumDiagnostics(cdoc.tu);

        if (diag_count > 0) {
            for (unsigned i = 0; i < diag_count; i += 1) {
                CXDiagnostic diagnostic = clang_getDiagnostic(cdoc.tu, i);
                CXSourceLocation location = clang_getDiagnosticLocation(diagnostic);

                CXFile file;
                unsigned line, column, offset;
                clang_getSpellingLocation(location, &file, &line, &column, &offset);
                CXString text = clang_getDiagnosticSpelling(diagnostic);
                CXString file_name = clang_getFileName(file);
                fprintf(stderr, "%s line %u, column %u: %s\n", clang_getCString(file_name),
                        line, column, clang_getCString(text));
            }

            return 1;
        }
    }

    CXCursor cursor = clang_getTranslationUnitCursor(cdoc.tu);

    clang_visitChildren(cursor, visitor, &cdoc);

    return 0;
}
