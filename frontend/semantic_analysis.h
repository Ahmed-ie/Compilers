/*
 * Semantic analysis for MiniC.
 */
#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include "ast.h"

// Returns true if an error is found, false otherwise.
bool semanticAnalysis(astNode *node);

#endif
