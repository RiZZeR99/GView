#include "js.hpp"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <uchar.h>
#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
using namespace std;

namespace GView::Type::JS::Plugins
{
using namespace GView::View::LexicalViewer;

std::string_view EliminateComments::GetName()
{
    return "Eliminate comments";
}
std::string_view EliminateComments::GetDescription()
{
    return "Eliminates comments from a JS scirpt.";
}

bool EliminateComments::CanBeAppliedOn(const GView::View::LexicalViewer::PluginData& data)
{
    //at least one token is of type comment
    for (auto index = data.startIndex; index < data.endIndex; index++)
    {
        if (data.tokens[index].GetTypeID(TokenType::None) == TokenType::Comment)
        {
            return true;
        }
    }
   return false;
}
GView::View::LexicalViewer::PluginAfterActionRequest EliminateComments::Execute(GView::View::LexicalViewer::PluginData& data)
{
    ofstream debug_file("debug.txt");
    //count how many characters were removed in order to know the start offset of a token in the data after removing a comment
    int32 count_removed_characters = 0; 

    for (int32 index = (int32) data.startIndex; index < (int32) data.endIndex; index++)
    {
        Token currentToken = data.tokens[index];
        
        if (currentToken.IsValid() && currentToken.GetTypeID(TokenType::None) == TokenType::Comment)
        {
            auto startOffset = currentToken.GetTokenStartOffset();
            auto endOffset   = currentToken.GetTokenEndOffset();
            if (!startOffset.has_value() || !endOffset.has_value())
                return GView::View::LexicalViewer::PluginAfterActionRequest::None;
            auto size = endOffset.value() - startOffset.value();
            
            debug_file << "Size: " << size << " Start offset: " << startOffset.value()
                       << " End offset: " << endOffset.value() << " Current index: " << index << "\n";

           data.editor.Replace(startOffset.value()-count_removed_characters, size, "");
            count_removed_characters += size;
            
        }
    }

    return GView::View::LexicalViewer::PluginAfterActionRequest::Rescan;
}
} // namespace GView::Type::JS::Plugins