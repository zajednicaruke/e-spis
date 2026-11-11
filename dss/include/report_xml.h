#ifndef REPORT_XML_H
#define REPORT_XML_H

#include <stdio.h>
#include <stdint.h>

typedef enum {
    NODE_TYPE_CATEGORY = 0,
    NODE_TYPE_QUESTION = 1,
    NODE_TYPE_ACTION  = 2
} NodeType;

typedef struct DiagnosticNode {
    int id;
    NodeType type;
    char code[32];
    char description[256];
    struct DiagnosticNode* left;
    struct DiagnosticNode* right;
} DiagnosticNode;

DiagnosticNode* create_node(int id, NodeType type, const char* code, const char* desc);
void generate_diagnostic_xml(const char* filename, const DiagnosticNode* root);
void free_tree(DiagnosticNode* node);

#endif // REPORT_XML_H
