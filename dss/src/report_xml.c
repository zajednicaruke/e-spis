#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "report_xml.h"

DiagnosticNode* create_node(int id, NodeType type, const char* name, const char* value) {
    DiagnosticNode* node = (DiagnosticNode*)malloc(sizeof(DiagnosticNode));
    if (!node) return NULL;
    node->id = id;
    node->type = type;
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    strncpy(node->value, value, sizeof(node->value) - 1);
    node->value[sizeof(node->value) - 1] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_tree(DiagnosticNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

static void print_xml_node(FILE *f, DiagnosticNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) fprintf(f, "  ");
    fprintf(f, "<DiagnosticNode id=\"%d\" type=\"%d\" name=\"%s\">\n", node->id, node->type, node->name);

    for (int i = 0; i < indent + 2; i++) fprintf(f, "  ");
    // CDATA sekvenca u potpunosti očuvala UTF-8 hrvatske dijakritike (č, ć, đ, š, ž)
    fprintf(f, "<Value><![CDATA[%s]]></Value>\n", node->value);

    if (node->left) print_xml_node(f, node->left, indent + 2);
    if (node->right) print_xml_node(f, node->right, indent + 2);

    for (int i = 0; i < indent; i++) fprintf(f, "  ");
    fprintf(f, "</DiagnosticNode>\n");
}

int generate_diagnostic_xml(const char* filename, DiagnosticNode* root) {
    FILE *f = fopen(filename, "w");
    if (!f) return 0;

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<DiagnosticTree xmlns=\"http://dss.esig.europa.eu/diagnostic/tree\">\n");
    fprintf(f, "  <!-- Institut Zajednice RUKE (OIB: 64517735227) — Službeno DSS Izvješće -->\n");

    print_xml_node(f, root, 2);

    fprintf(f, "</DiagnosticTree>\n");
    fclose(f);
    return 1;
}
