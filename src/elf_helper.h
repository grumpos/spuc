#pragma once
#ifndef __ELF_HELPER_H__
#define __ELF_HELPER_H__

#include "elf.h"
#include <memory>

using std::string;
using std::vector;

struct range_t
{
	inline size_t		size() const	{ return length_; }
	inline void*		begin()			{ return data_; }
	inline const void*	begin() const	{ return data_; }
	inline void*		end()			{ return (void*)((char*)data_ + length_); }
	inline const void*	end() const		{ return (void*)((char*)data_ + length_); }

	void* data_;
	size_t length_;
};


struct ELF_32_traits_t
{
	typedef Elf32_Ehdr header_type;
	typedef Elf32_Phdr pheader_type;
	typedef Elf32_Shdr sheader_type;
	typedef Elf32_Sym  symbol_type;

	static const char* header_layout;
	static const char* pheader_layout;
	static const char* sheader_layout;
	static const char* symbol_layout;
};

const char* ELF_32_traits_t::header_layout	= "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\2\2\4\4\4\4\4\2\2\2\2\2\2";
const char* ELF_32_traits_t::pheader_layout = "\4\4\4\4\4\4\4\4";
const char* ELF_32_traits_t::sheader_layout = "\4\4\4\4\4\4\4\4\4\4";
const char* ELF_32_traits_t::symbol_layout	= "\4\4\4\1\1\2";

struct ELF_64_traits_t
{
	typedef Elf64_Ehdr header_type;
	typedef Elf64_Phdr pheader_type;
	typedef Elf64_Shdr sheader_type;
	typedef Elf64_Sym  symbol_type;

	static const char* header_layout;
	static const char* pheader_layout;
	static const char* sheader_layout;
	static const char* symbol_layout;
};

const char* ELF_64_traits_t::header_layout	= "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\2\2\4\x8\x8\x8\4\2\2\2\2\2\2";
const char* ELF_64_traits_t::pheader_layout = "\4\4\x8\x8\x8\x8\x8\x8";
const char* ELF_64_traits_t::sheader_layout = "\4\4\x8\x8\x8\x8\4\4\x8\x8";
const char* ELF_64_traits_t::symbol_layout	= "\4\1\1\2\x8\x8";

template<class T>
struct ByteSwapHelper : public std::unary_function<T, void>
{
	std::string LayoutString_;
	ByteSwapHelper( const std::string& LayoutString )
		: LayoutString_(LayoutString)
	{}

	void operator()( argument_type& Data ) const
	{
		ReverseBytesInRange( &Data, LayoutString_.c_str() );
	}
};


template <class ELFTraits>
class ElfFile
{
public:	
	typedef typename ELFTraits::header_type		header_type;
	typedef typename ELFTraits::pheader_type	pheader_type;
	typedef typename ELFTraits::sheader_type	sheader_type;
	typedef typename ELFTraits::symbol_type		symbol_type;

	string GetSHName( size_t index ) { return index < SHNames_.size() ? SHNames_[index] : ""; }
	string GetSymbolName( size_t index ) { return index < StringTBL_.size() ? StringTBL_[index] : ""; }

	explicit ElfFile( const char* FilePath, size_t OffsetInFile = 0 )
		: raw_file_(open(FilePath)), OffsetInFile_(OffsetInFile)
	{
		_ReadELFHeader();
		_ReadProgramHeaders();
		_ReadSectionHeaders();
		_ReadRawProgramData();
		_ReadRawSectionData();		
		_ReadSectionNames();
		
		_ReadStringTable();
		_ReadSymbolTable();
		//close(raw_file_);
		return;
		/*
		std::vector<symbol_type> funtion_symbols;
		
		std::copy_if( Symbols.begin(), Symbols.end(), std::back_inserter(funtion_symbols), []( symbol_type& s ) -> bool { 
			const bool funtion_related = (ELF32_ST_TYPE( s.st_info ) == STT_FUNC) 
				&& (ELF32_ST_BIND( s.st_info ) == STB_GLOBAL);
			return funtion_related;
		});

		std::vector<string> fun_names;


		std::transform( funtion_symbols.begin(), funtion_symbols.end(), std::back_inserter( fun_names ),
			[&]( symbol_type& s ) -> string
		{ 
			return StringTBL_[s.st_name];
		});


		std::vector<range_t> fun_bytecodes;
		std::transform( funtion_symbols.begin(), funtion_symbols.end(), std::back_inserter( fun_bytecodes ),
			[&]( symbol_type& s ) -> range_t
		{ 
			const sheader_type& owner_sh = SHBegin()[s.st_shndx];
			const size_t fn_offset_in_sh = s.st_value - owner_sh.sh_addr;

			range_t res;
			res.data_ = ((char*)begin(raw_file_)) + owner_sh.sh_offset + fn_offset_in_sh;
			res.length_ = s.st_size;
			return res;
		});

		assert( fun_bytecodes.size() == fun_names.size() );
		
		std::ofstream fout("spu_cpp.cpp");
		auto fun_sym_iter = funtion_symbols.cbegin();
		auto fun_name_iter = fun_names.cbegin();

		std::for_each( fun_bytecodes.begin(), fun_bytecodes.end(),
			[&]( const range_t& bc )
		{
			std::list<string> fun_def;
			const size_t fun_base_address = (fun_sym_iter++)->st_value;

			const uint8_t* bytecode = (uint8_t*)bc.begin();


			[&fun_def] // Indent {} blocks
			{
				const string indent = "    ";
				std::stack<std::list<string>::iterator> left_brackets;

				for ( auto it = fun_def.begin(); it != fun_def.end(); ++it )
				{
					if ( *it == "{" )
					{
						left_brackets.push(it);
					}
					else if ( *it == "}"  )
					{
						assert(!left_brackets.empty());

						auto block_begin	= ++left_brackets.top();
						auto block_end		= it;

						std::transform( block_begin, block_end, block_begin,
							[&indent](string& s) { return indent + s; });

						left_brackets.pop();
					}
				}
			}();

			fout << "void " << *fun_name_iter++ << "()\n{\n";

			//std::copy( fun_def.begin(), fun_def.end(), std::ostream_iterator<string>(fout, "\n") );
			std::transform( fun_def.begin(), fun_def.end(), std::ostream_iterator<string>(fout, "\n"),
				[](string& s) { return string("\t") + s;});
			fout << "}\n\n\n";
			
		});
		*/
		/*std::vector<symbol_type> objects;
		std::copy_if( Symbols.begin(), Symbols.end(), std::back_inserter(objects), 
			[]( symbol_type& s ) -> bool
		{ 
			const bool funtion_related = (ELF32_ST_TYPE( s.st_info ) == STT_OBJECT) 
				&& (ELF32_ST_BIND( s.st_info ) == STB_GLOBAL);
			return funtion_related;
		});

		std::vector<string> object_names;
		std::transform( objects.begin(), objects.end(), std::back_inserter( object_names ),
			[&]( symbol_type& s ) -> string
		{ 
			return string( (const char*)SDBegin()[stbl_sh_index].begin() + s.st_name );
		});*/
	}
	~ElfFile()
	{
		close(raw_file_);
	}
	

	memmap_t* raw_file_;
	size_t OffsetInFile_;

	header_type					HeaderLE_;
	vector<pheader_type>		ProgramHeaders_;
	vector<sheader_type>		SectionHeaders_;
	vector<range_t>				PData_;
	vector<range_t>				SData_;
	vector<string>				SHNames_;
	std::map<uint32_t, string>	StringTBL_;
	std::vector<symbol_type>	Symbols;

	inline uint8_t* GetELFFileBegin() { return (uint8_t*)begin(raw_file_) + OffsetInFile_; }

	void _ReadELFHeader()
	{
		HeaderLE_ = *(header_type*)GetELFFileBegin();
		ReverseBytesInRange( &HeaderLE_, ELFTraits::header_layout );
	}

	void _ReadProgramHeaders()
	{
		const pheader_type* ph_b = (pheader_type*)(GetELFFileBegin() + HeaderLE_.e_phoff);
		const pheader_type* ph_e = ph_b + HeaderLE_.e_phnum;

		ProgramHeaders_.resize(HeaderLE_.e_phnum);
		std::copy( ph_b, ph_e, ProgramHeaders_.begin() );

		std::for_each( ProgramHeaders_.begin(), ProgramHeaders_.end(), ByteSwapHelper<pheader_type>(ELFTraits::pheader_layout) );
	}

	void _ReadSectionHeaders()
	{
		sheader_type* sh_b = (sheader_type*)(GetELFFileBegin() + HeaderLE_.e_shoff);
		sheader_type* sh_e = sh_b + HeaderLE_.e_shnum;

		SectionHeaders_.resize(HeaderLE_.e_shnum);
		std::copy( sh_b, sh_e, SectionHeaders_.begin() );

		std::for_each( SectionHeaders_.begin(), SectionHeaders_.end(), ByteSwapHelper<sheader_type>(ELFTraits::sheader_layout) );
	}

	void _ReadRawProgramData()
	{
		std::for_each( ProgramHeaders_.begin(), ProgramHeaders_.end(), 
			[&]( const pheader_type& p )
		{ 
			range_t fdata;
			fdata.data_		= GetELFFileBegin() + p.p_offset;
			fdata.length_	= p.p_filesz;

			PData_.push_back(fdata);
		} );
	}

	void _ReadRawSectionData()
	{
		std::for_each( SectionHeaders_.begin(), SectionHeaders_.end(), 
			[&]( const sheader_type& s )
		{ 
			range_t fdata;
			fdata.data_		= GetELFFileBegin() + s.sh_offset;
			fdata.length_	= s.sh_size;

			SData_.push_back(fdata);
		} );
	}

	void _ReadSectionNames()
	{
		if ( 0 != HeaderLE_.e_shstrndx )
		{
			const char* SectionNameStringTableBase = (const char*)SData_[HeaderLE_.e_shstrndx].begin();

			std::transform( SectionHeaders_.begin(), SectionHeaders_.end(), std::back_inserter( SHNames_ ), 
				[SectionNameStringTableBase]( const sheader_type& sh )->string
			{
				return string( SectionNameStringTableBase + sh.sh_name );
			});
		}		
	}

	void _ReadStringTable()
	{
		const size_t strtab_index = std::distance( 
			SHNames_.begin(), 
			std::find( SHNames_.begin(), SHNames_.end(), ".strtab" ) );

		if ( strtab_index != SHNames_.size() )
		{			
			const char* StringTableBase		= (const char*)SData_[strtab_index].begin();
			const size_t StringTableLength	= SectionHeaders_[strtab_index].sh_size;
			const size_t NullTermCharLength = 1;

			for ( size_t BytesRead = 0; BytesRead < StringTableLength; )
			{
				string temp( StringTableBase + BytesRead );
				StringTBL_[BytesRead] = temp;
				BytesRead += (temp.size() + NullTermCharLength);
			}
		}
	}

	void _ReadSymbolTable()
	{
		size_t symtab_index = std::distance( 
			SHNames_.begin(), 
			std::find( SHNames_.begin(), SHNames_.end(), ".symtab" ) );

		if ( symtab_index != SHNames_.size() )
		{
			std::copy( (symbol_type*)SData_[symtab_index].begin(), 
				(symbol_type*)SData_[symtab_index].end(), std::back_inserter( Symbols ) );

			std::for_each( Symbols.begin(), Symbols.end(), ByteSwapHelper<symbol_type>(ELFTraits::symbol_layout) );
		}		
	}
};

#endif __ELF_HELPER_H__