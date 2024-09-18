
#ifndef LOCALIZATION_USE_DEFAULT_LOCAL
    #define LOCALIZATION_USE_DEFAULT_LOCAL false
#endif

#ifndef LOCALIZATION_DEFAULT_LOCAL
    #define LOCALIZATION_DEFAULT_LOCAL fixed_string("en")
#endif

#ifndef LOCALIZATION_USE_PLACEHOLDER_STRING
    #define LOCALIZATION_USE_PLACEHOLDER_STRING false
#endif

typedef u32 Localized_Text;

typedef struct Localized_Text_Table {
    string * values;
    u32 count;
} LocalizedTextTable;

Hash_Table localized_text;

void initialize_localization() {
    localized_text = make_hash_table(string, LocalizedTextTable, get_heap_allocator());
}

bool utf32_is_white_space(u32 character) {
    static const u32 white_space_characters[] = {0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x0085, 0x00A0, 0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x200B, 0x200C, 0x200D, 0x2028, 0x2029, 0x202F, 0x205F, 0x2060, 0x3000, 0xFEFF};
    u32 white_space_count = sizeof(white_space_characters);

    for(int i = 0; i < white_space_count; i++) {
        if(character == white_space_characters[i]) {
            return true;
        }
    }
    return false;
}

// Loads localized text from a string, each string is seperated by a new line.
bool load_localized_text_from_string(string text, string local) {   
    assert(text.count);

    LocalizedTextTable new_text_table = {};

    // Count the number of lines in the file skipping lines that only contain white space.
    string loop_text = text;
    u32 non_white_space_count = 0;
    for(u32 character = next_utf8(&loop_text); character !=0 ; character = next_utf8(&loop_text)) {
        if(character == '\n' || character == '\r') {
            if(non_white_space_count > 0) new_text_table.count++;
            non_white_space_count = 0;
        } else if (!utf32_is_white_space(character)){
            non_white_space_count++;
        }
    }

    // If the last character of the file is not a new line
    // We need to add one to the string count so we process
    // that line of text.
    if(text.data[text.count-1] != '\n' && text.data[text.count-1] != '\r') {
        new_text_table.count++;
    }

    assert(new_text_table.count);

    // Deallocate up the existing table if we replacing it
    // This is so we can do hot loading of our localization files later.
    LocalizedTextTable * previous_text = hash_table_find(&localized_text, local);
    if(previous_text) {
        dealloc(get_heap_allocator(), previous_text->values);
    }
    
    new_text_table.values = alloc(get_heap_allocator(), sizeof(string) * new_text_table.count);
    u32 current_character_index = 0;
    for(u32 i = 0; i < new_text_table.count; i++) {
        new_text_table.values[i].data = &text.data[current_character_index];
        while(current_character_index < text.count) {
            Utf8_To_Utf32_Result result = utf8_to_utf32(&text.data[current_character_index], text.count-current_character_index, false);
            
            current_character_index += result.continuation_bytes;
            assert(current_character_index <= text.count);

            if(result.utf32 == '\n' || result.utf32 == '\r') {
                
                result = utf8_to_utf32(&text.data[current_character_index], text.count-current_character_index, false);
                while(result.utf32 == '\n' || result.utf32 == '\r') {
                    if(current_character_index < text.count) {
                        current_character_index += result.continuation_bytes;
                        assert(current_character_index <= text.count);
                        result = utf8_to_utf32(&text.data[current_character_index], text.count-current_character_index, false);
                    }
                }
                
                break;
            };

            new_text_table.values[i].count += result.continuation_bytes;
        }
    }

    hash_table_set(&localized_text, local, new_text_table);
    return true;
}

// Loads localized text from a file, each string is seperated by a new line
bool load_localized_text_from_file(string filename, string local) {
    string file_text;
    if(!os_read_entire_file_s(filename, &file_text, get_heap_allocator())) return false;
    return load_localized_text_from_string(file_text, local);
}

string get_localized_text(Localized_Text text, string local) {
    LocalizedTextTable * text_table = hash_table_find(&localized_text, local);
    if(!text_table) {

#if LOCALIZATION_USE_DEFAULT_LOCAL
        
            text_table = hash_table_find(&localized_text, LOCALIZATION_DEFAULT_LOCAL);
            assert(text_table);

#else

            return tprintf("!!! UNSUPORTED OR INVALID LOCAL %s !!!", local);

#endif

    }


#if LOCALIZATION_USE_PLACEHOLDER_STRING

    if(text == 0 || text > text_table->count) {
        return tprintf("!!! INVALID TEXT ID %i !!!", text);

    }
    
#else

    assert(text != 0 && text <= text_table->count);

#endif
    
    return text_table->values[text-1];
}