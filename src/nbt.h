#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <array>
#include <iostream>
#include <memory>
#include <zlib.h>

using std::variant;
using byte = unsigned char;

const unsigned int HEADER_SIZE = 4;

namespace nbt {
	enum NbtTagType : uint8_t {
		TAG_END,
		TAG_Byte,
		TAG_Short,
		TAG_Int,
		TAG_Long,
		TAG_Float,
		TAG_Double,
		TAG_Byte_Array,
		TAG_String,
		TAG_List,
		TAG_Compound,
		TAG_Int_Array,
		TAG_Long_Array
	};


	/// <summary>
	/// Represents an NBT node. Payload is a union so check the type before trying
	/// to access!
	/// </summary>
	struct nbt_node {
        struct compound {
            std::vector<nbt_node> content;

            /// Gets the child node with name `key`
            /// \param key
            /// \return returns `nullptr` if no child with name `key` could be found
            const nbt_node* operator[](const std::string& key) const {
                auto found = std::find_if(content.begin(), content.end(),
                        [&key](const nbt_node el) { return el.name == key; }
                        );

                if (found == content.end()) return nullptr;

                return &*found;
            }

            /// Gets the child node with name `key`
            /// \param key
            /// \return returns `nullptr` if no child with name `key` could be found
            nbt_node* operator[](const std::string& key) {
                return const_cast<nbt_node*>(static_cast<compound>(*this)[key]);
            }

            template<class T>
            void insert_node(T const& value, std::string const& name) {
                nbt_node node{value};
                node.name = name;
                content.push_back(node);
            }
                        
            std::vector<nbt_node>::iterator begin() { return content.begin(); }
            std::vector<nbt_node>::iterator end() { return content.end(); }
        };

        typedef variant<
                byte,
                int16_t,
                int32_t,
                int64_t,
                float,
                double,
                std::vector<byte>,
                std::string,
                std::vector<nbt_node>,
                compound,
                std::vector<int32_t>,
                std::vector<int64_t>
            > payload_t;

		template<typename T>
		T& operator[](int index);

        template<uint8_t I>
        const auto& get() const {
            return std::get<I-1>(payload);
        }

        template<uint8_t I>
        auto& get(){
            return std::get<I-1>(payload);
        }

		nbt_node() : tagtype(NbtTagType::TAG_END) { }
		explicit nbt_node(NbtTagType t_) : tagtype(t_) { }
        nbt_node(int32_t i) : tagtype(TAG_Int), payload(i) { }
        nbt_node(int64_t l) : tagtype(TAG_Long), payload(l) { }
        nbt_node(float f) : tagtype(TAG_Float), payload(f) { }
        nbt_node(double d) : tagtype(TAG_Double), payload(d) { }
        nbt_node(std::vector<byte> b_array) : tagtype(TAG_Byte_Array), payload(b_array) { }
        nbt_node(std::vector<int32_t> i32_array) : tagtype(TAG_Int_Array), payload(i32_array) { }
        nbt_node(std::vector<int64_t> i64_array) : tagtype(TAG_Long_Array), payload(i64_array) { }

		std::string pretty_print(uint16_t level = 0) const;

		nbt_node* operator[](const std::string& name);
        nbt_node* at(const std::string& name);

        payload_t payload;
		std::string name;
		NbtTagType tagtype;
	};

	//nbt_node& get_child(const std::string& name, const nbt_node& parent);

//	std::vector<nbt_node*> load_region_fopen(const char* filename);

#pragma region I/O

    /// Load region in anvil format
    /// \param filename
    /// \return
	std::vector<nbt_node> load_region(const char* filename);

    /// Load an nbt_node from file
    nbt_node read_from_file(std::string const& filename);

    /// Write file
    void write_to_file(nbt_node const& node, std::string const& filename);

    /// Read node from a byte buffer (e.g. from ifstream or zlib)
    /// \param buffer
    /// \return
	nbt_node read_node(const char* &buffer);

    /// Write node to buffer
    /// \param node
    /// \param buffer
    void write_node(const nbt_node& node, std::vector<char>& buffer);

#pragma endregion

#pragma region Internals

    /// (internal) read name from buffer
	std::string get_name(const char* &buffer);

    /// (internal) convert payload
    /// \param id
    /// \param buffer
    /// \param node
	void get_payload(NbtTagType id,
		const char* &buffer,
		nbt_node* node);

    struct plevel {
        uint16_t l;
    };
#pragma endregion
}

inline std::ostream& operator<<(std::ostream& os, const nbt::plevel& l) {
    for(uint16_t i = 0; i < l.l-1; i++)
    {
        os.put(' ');
        os.put(' ');
    }
    os.put(' ');
    os.put(' ');
    return os;
}

std::ostream& operator<<(std::ostream& os, const nbt::nbt_node& n);
