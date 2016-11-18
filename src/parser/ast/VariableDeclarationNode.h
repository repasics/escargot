/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef VariableDeclarationNode_h
#define VariableDeclarationNode_h

#include "DeclarationNode.h"
#include "VariableDeclaratorNode.h"

namespace Escargot {

class VariableDeclarationNode : public DeclarationNode {
public:
    friend class ScriptParser;
    VariableDeclarationNode(VariableDeclaratorVector&& decl)
        : DeclarationNode()
    {
        m_declarations = decl;
    }

    virtual ASTNodeType type() { return ASTNodeType::VariableDeclaration; }
    VariableDeclaratorVector& declarations() { return m_declarations; }
protected:
    VariableDeclaratorVector m_declarations; // declarations: [ VariableDeclarator ];
    // kind: "var" | "let" | "const";
};

}

#endif
