#ifndef REPORT_XML_H
#define REPORT_XML_H

#include <stdio.h>

typedef enum {
    NODE_OK = 0,
    NODE_WARNING,
    NODE_ERROR
} NodeType;

typedef struct DiagnosticNode {
    int id;
    NodeType type;
    char name[64];
    char value[512];
    struct DiagnosticNode *left;
    struct DiagnosticNode *right;
} DiagnosticNode;

DiagnosticNode* create_node(int id, NodeType type, const char* name, const char* value);
void free_tree(DiagnosticNode* root);
void generate_diagnostic_xml(const char* filename, DiagnosticNode* root);

#endif // REPORT_XML_H
