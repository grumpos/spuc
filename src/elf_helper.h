#ifndef __ELF_HELPER_H__
#define __ELF_HELPER_H__

#include <string>
#include <map>
#include <vector>

#include "elf.h"
#include "raw.h"
#include "tools.h"

#include <memory>

using std::string;
using std::vector;
using std::map;
using std::shared_ptr;

struct memrange_t
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

typedef ELF_32_traits_t SPU_ELF;
typedef ELF_64_traits_t PPU_ELF;

template <class ELFTraits>
class ElfFile
{
public:	
	typedef typename ELFTraits::header_type		header_type;
	typedef typename ELFTraits::pheader_type	pheader_type;
	typedef typename ELFTraits::sheader_type	sheader_type;
	typedef typename ELFTraits::symbol_type		symbol_type;

public:
	shared_ptr<memmap_t>		RawFile_;
	uint8_t*					ELFBase_;

	header_type					HeaderLE_;
	vector<pheader_type>		ProgramHeaders_;
	vector<sheader_type>		SectionHeaders_;
	vector<memrange_t>			PData_;
	vector<memrange_t>			SData_;
	vector<string>				SHNames_;
	map<size_t, string>			StringTBL_;
	vector<symbol_type>			Symbols_;

public:
	string GetSHName( size_t index ) const { return index < SHNames_.size() ? SHNames_[index] : ""; }
	string GetSymbolName( size_t index ) const { return index < StringTBL_.size() ? StringTBL_[index] : ""; }

	explicit ElfFile( const char* FilePath, size_t OffsetInFile = 0 )
		: RawFile_(shared_ptr<memmap_t>(mmopen(FilePath), mmclose)),
		ELFBase_((uint8_t*)mmbegin(RawFile_.get()) + OffsetInFile)
	{
		_ParseELF();
	}

	// TODO: make it parse from an arbitrary pointer
	//explicit ElfFile( const void* FileData )
	//	: RawFile_(shared_ptr<memmap_t>(mmopen(FilePath.c_str()), mmclose)),
	//	ELFBase_((uint8_t*)mmbegin(RawFile_.get()) + OffsetInFile)
	//{
	//	_ParseELF();
	//}

	~ElfFile()
	{
	}

private:
	void _ParseELF()
	{
		_ReadELFHeader();

		_ReadProgramHeaders();
		_ReadSectionHeaders();

		_ReadRawProgramData();
		_ReadRawSectionData();	

		_ReadSectionNames();		
		_ReadStringTable();
		_ReadSymbolTable();
	}

	void _ReadELFHeader()
	{
		HeaderLE_ = *(header_type*)ELFBase_;
		ReverseBytesInRange( &HeaderLE_, ELFTraits::header_layout );
	}

	void _ReadProgramHeaders()
	{
		const pheader_type* ph_b = (pheader_type*)(ELFBase_ + HeaderLE_.e_phoff);
		const pheader_type* ph_e = ph_b + HeaderLE_.e_phnum;

		ProgramHeaders_.assign(ph_b, ph_e);

		for (auto& ph : ProgramHeaders_)
		{
			ReverseBytesInRange(&ph, ELFTraits::pheader_layout);
		}
	}

	void _ReadSectionHeaders()
	{
		const sheader_type* sh_b = (sheader_type*)(ELFBase_ + HeaderLE_.e_shoff);
		const sheader_type* sh_e = sh_b + HeaderLE_.e_shnum;

		SectionHeaders_.assign(sh_b, sh_e);

		for (auto& sh : SectionHeaders_)
		{
			ReverseBytesInRange(&sh, ELFTraits::sheader_layout);
		}
	}

	void _ReadRawProgramData()
	{
		for (auto& ph : ProgramHeaders_)
		{ 
			memrange_t fdata;
			fdata.data_		= ELFBase_ + ph.p_offset;
			fdata.length_	= ph.p_filesz;

			PData_.push_back(fdata);
		}
	}

	void _ReadRawSectionData()
	{
		for (auto& sh : SectionHeaders_)
		{ 
			memrange_t fdata;
			fdata.data_		= ELFBase_ + sh.sh_offset;
			fdata.length_	= sh.sh_size;

			SData_.push_back(fdata);
		}
	}

	void _ReadSectionNames()
	{
		if ( 0 != HeaderLE_.e_shstrndx )
		{
			const char* SectionNameSTBL = (const char*)SData_[HeaderLE_.e_shstrndx].begin();

			SHNames_.reserve(SectionHeaders_.size());

			for(auto& sh : SectionHeaders_)
			{
				SHNames_.push_back(string( SectionNameSTBL + sh.sh_name ));
			}
		}		
	}

	void _ReadStringTable()
	{
		const size_t strtab_index = std::distance( 
			SHNames_.begin(), 
			std::find( SHNames_.begin(), SHNames_.end(), ".strtab" ) );

		if ( strtab_index != SHNames_.size() )
		{			
			const char* StringTableBase	= (const char*)SData_[strtab_index].begin();
			const char* Str	= StringTableBase + 1;

			while (*Str)
			{
				string StrEntry(Str);
				StringTBL_[Str - StringTableBase] = StrEntry;
				Str += StrEntry.size() + 1;
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
				(symbol_type*)SData_[symtab_index].end(), std::back_inserter( Symbols_ ) );

			std::for_each( Symbols_.begin(), Symbols_.end(), [](symbol_type& sym)
			{
				ReverseBytesInRange(&sym, ELFTraits::symbol_layout);
			});
		}		
	}
};

#endif __ELF_HELPER_H__