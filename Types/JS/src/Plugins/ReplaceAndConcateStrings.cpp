#include "js.hpp";
//#include "../../../../GViewCore/src/View/LexicalViewer/LexicalViewer.hpp"
//#include "../../../../GViewCore/src/View/LexicalViewer/Instance.cpp"

#include<map>
#include<list>

namespace GView::Type::JS::Plugins
{
using namespace GView::View::LexicalViewer;

std::string_view ReplaceAndConcateStrings::GetName()
{
    return "ReplaceAndConcateStrings";
}
std::string_view ReplaceAndConcateStrings::GetDescription()
{
    return "Replace and concatenate string as much as possible.";
}

bool ReplaceAndConcateStrings::CanBeAppliedOn(const GView::View::LexicalViewer::PluginData& data)
{
    for (auto index = data.startIndex; index < data.endIndex; index++)
    {
        if ((data.tokens[index].GetTypeID(TokenType::None) == TokenType::String &&
            data.tokens[index].Precedent().GetTypeID(TokenType::None) == TokenType::Operator_Plus) ||
                (data.tokens[index].GetTypeID(TokenType::None) == TokenType::Operator_Plus&&
             data.tokens[index].Precedent().GetTypeID(TokenType::None) == TokenType::String))
        {
            return true;
        }
    }
    return false;
}

void ConcatenateStrings(GView::View::LexicalViewer::PluginData& data)
{
    int32 index = (int32) data.endIndex - 1;
    LocalUnicodeStringBuilder<256> temp;
    while (index >= (int32) data.startIndex)
    {
        Token endToken = data.tokens[index];
        if (endToken.GetTypeID(TokenType::None) == TokenType::String &&
            endToken.Precedent().GetTypeID(TokenType::None) == TokenType::Operator_Plus &&
            endToken.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::String)
        {
            Token start = endToken.Precedent().Precedent();
            while (start.Precedent().GetTypeID(TokenType::None) == TokenType::Operator_Plus &&
                   start.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::String)
            {
                start = start.Precedent().Precedent();
            }
            temp.Clear();
            temp.AddChar('"');
            index            = start.GetIndex();
            auto startOffset = start.GetTokenStartOffset();
            auto endOffset   = endToken.GetTokenEndOffset();
            if (!startOffset.has_value() || !endOffset.has_value())
                return;
            auto size = endOffset.value() - startOffset.value();
            while (start.GetIndex() <= endToken.GetIndex())
            {
                auto txt   = start.GetText();
                auto value = txt.substr(1, txt.length() - 2);
                if (value.find_first_of('"') == std::u16string_view::npos)
                {
                    temp.Add(value);
                }
                else
                {
                    for (auto ch : value)
                    {
                        if (ch == '"')
                            temp.AddChar('\\');
                        temp.AddChar(ch);
                    }
                }
                // temp.Add(txt.substr(1, txt.length() - 2));
                start = start.Next().Next();
            }
            temp.AddChar('"');
            data.editor.Replace(startOffset.value(), size, temp.ToStringView());
        }
        index--;
    }
}

GView::View::LexicalViewer::PluginAfterActionRequest ReplaceAndConcateStrings::Execute(GView::View::LexicalViewer::PluginData& data)
{
    
    int32 indexToken = 0;

    std::map<u16string_view, u16string_view> variables;
    LocalUnicodeStringBuilder<256> variableNameHandler;

    while (indexToken < data.endIndex)
    {
        Token token = data.tokens[indexToken];
        if (token.GetTypeID(TokenType::None) == TokenType::DataType_Var) // declaring a variable
        {
            variableNameHandler.Clear();
            Token varName = data.tokens[indexToken + 1]; //getting the name of the variable for further storage
            for (char ch : varName.GetText())
            {
                variableNameHandler.AddChar(ch);
            }

            if (data.tokens[indexToken + 2].GetTypeID(TokenType::None) == TokenType::Operator_Assignment)//checking if the variable is simple initialized
            {
                std::list<u16string_view> valuesForVariable;
                bool simpleInitialization = true;
                int32 temp_index = indexToken+3;
                auto start                = data.tokens[temp_index].GetTokenStartOffset().value();
                while (temp_index < data.endIndex && data.tokens[temp_index].GetTypeID(TokenType::None) != TokenType::Semicolumn)
                {
                    Token temp_token = data.tokens[temp_index];
                    if (temp_token.GetTypeID(TokenType::None) == TokenType::String)
                    {
                        valuesForVariable.push_back(temp_token.GetText());
                        temp_index++;
                    }
                    else if (temp_token.GetTypeID(TokenType::None) == TokenType::Operator_Plus)
                    {
                        temp_index++;
                    }
                    else if (temp_token.GetTypeID(TokenType::None) != TokenType::Semicolumn)
                    {
                        simpleInitialization = false;
                        break;
                    }
                }
                if (simpleInitialization)
                {
                    LocalUnicodeStringBuilder<256> valueBuilder;
                    valueBuilder.Clear();
                    for (u16string_view string: valuesForVariable)
                    {
                        for (char ch : string)
                        {
                            valueBuilder.AddChar(ch);
                        }
                    }
                    variables[varName.GetText()] = valueBuilder.ToStringView();
                    data.editor.Replace(
                          start, data.tokens[temp_index].GetTokenEndOffset().value() - start-1, variables[varName.GetText()]);
                }
                else
                {
                    // another variable was used for declaring the current one
                    valuesForVariable.push_back(variables[data.tokens[temp_index].GetText()]);
                }
            }
        }
        indexToken++;
    }
    ConcatenateStrings(data);
    return GView::View::LexicalViewer::PluginAfterActionRequest::Rescan;
}
} // namespace GView::Type::JS::Plugins