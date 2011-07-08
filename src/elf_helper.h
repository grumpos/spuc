#ifndef __ELF_HELPER_H__
#define __ELF_HELPER_H__
//
//#include "elf.h"
//#include "raw.h"
//#include <string>
//#include <map>
#include <vector>
////#include <memory>
//
//using std::string;
//using std::vector;
//
//struct memrange_t
//{
//	inline size_t		size() const	{ return length_; }
//	inline void*		begin()			{ return data_; }
//	inline const void*	begin() const	{ return data_; }
//	inline void*		end()			{ return (void*)((char*)data_ + length_); }
//	inline const void*	end() const		{ return (void*)((char*)data_ + length_); }
//
//	void* data_;
//	size_t length_;
//};
//
//
//struct ELF_32_traits_t
//{
//	typedef Elf32_Ehdr header_type;
//	typedef Elf32_Phdr pheader_type;
//	typedef Elf32_Shdr sheader_type;
//	typedef Elf32_Sym  symbol_type;
//
//	static const char* header_layout;
//	static const char* pheader_layout;
//	static const char* sheader_layout;
//	static const char* symbol_layout;
//};
//
//const char* ELF_32_traits_t::header_layout	= "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\2\2\4\4\4\4\4\2\2\2\2\2\2";
//const char* ELF_32_traits_t::pheader_layout = "\4\4\4\4\4\4\4\4";
//const char* ELF_32_traits_t::sheader_layout = "\4\4\4\4\4\4\4\4\4\4";
//const char* ELF_32_traits_t::symbol_layout	= "\4\4\4\1\1\2";
//
//struct ELF_64_traits_t
//{
//	typedef Elf64_Ehdr header_type;
//	typedef Elf64_Phdr pheader_type;
//	typedef Elf64_Shdr sheader_type;
//	typedef Elf64_Sym  symbol_type;
//
//	static const char* header_layout;
//	static const char* pheader_layout;
//	static const char* sheader_layout;
//	static const char* symbol_layout;
//};
//
//const char* ELF_64_traits_t::header_layout	= "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\2\2\4\x8\x8\x8\4\2\2\2\2\2\2";
//const char* ELF_64_traits_t::pheader_layout = "\4\4\x8\x8\x8\x8\x8\x8";
//const char* ELF_64_traits_t::sheader_layout = "\4\4\x8\x8\x8\x8\4\4\x8\x8";
//const char* ELF_64_traits_t::symbol_layout	= "\4\1\1\2\x8\x8";
//
//template<class T>
//struct ByteSwapHelper : public std::unary_function<T, void>
//{
//	std::string LayoutString_;
//	ByteSwapHelper( const std::string& LayoutString )
//		: LayoutString_(LayoutString)
//	{}
//
//	void operator()( argument_type& Data ) const
//	{
//		ReverseBytesInRange( &Data, LayoutString_.c_str() );
//	}
//};
//
//
//template <class ELFTraits>
//class ElfFile
//{
//public:	
//	typedef typename ELFTraits::header_type		header_type;
//	typedef typename ELFTraits::pheader_type	pheader_type;
//	typedef typename ELFTraits::sheader_type	sheader_type;
//	typedef typename ELFTraits::symbol_type		symbol_type;
//
//	string GetSHName( size_t index ) { return index < SHNames_.size() ? SHNames_[index] : ""; }
//	string GetSymbolName( size_t index ) { return index < StringTBL_.size() ? StringTBL_[index] : ""; }
//
//	explicit ElfFile( const char* FilePath, size_t OffsetInFile = 0 )
//		: raw_file_(open(FilePath)), OffsetInFile_(OffsetInFile)
//	{
//		_ReadELFHeader();
//
//		_ReadProgramHeaders();
//		_ReadSectionHeaders();
//
//		_ReadRawProgramData();
//		_ReadRawSectionData();	
//
//		_ReadSectionNames();		
//		_ReadStringTable();
//		_ReadSymbolTable();
//	}
//	~ElfFile()
//	{
//		close(raw_file_);
//	}
//	
//
//	memmap_t* raw_file_;
//	size_t OffsetInFile_;
//
//	header_type					HeaderLE_;
//	vector<pheader_type>		ProgramHeaders_;
//	vector<sheader_type>		SectionHeaders_;
//	vector<memrange_t>			PData_;
//	vector<memrange_t>			SData_;
//	vector<string>				SHNames_;
//	std::map<uint32_t, string>	StringTBL_;
//	std::vector<symbol_type>	Symbols;
//
//	inline uint8_t* GetELFFileBegin() { return (uint8_t*)begin(raw_file_) + OffsetInFile_; }
//
//	void _ReadELFHeader()
//	{
//		HeaderLE_ = *(header_type*)GetELFFileBegin();
//		ReverseBytesInRange( &HeaderLE_, ELFTraits::header_layout );
//	}
//
//	void _ReadProgramHeaders()
//	{
//		const pheader_type* ph_b = (pheader_type*)(GetELFFileBegin() + HeaderLE_.e_phoff);
//		const pheader_type* ph_e = ph_b + HeaderLE_.e_phnum;
//
//		ProgramHeaders_.resize(HeaderLE_.e_phnum);
//		std::copy( ph_b, ph_e, ProgramHeaders_.begin() );
//
//		std::for_each( ProgramHeaders_.begin(), ProgramHeaders_.end(), ByteSwapHelper<pheader_type>(ELFTraits::pheader_layout) );
//	}
//
//	void _ReadSectionHeaders()
//	{
//		sheader_type* sh_b = (sheader_type*)(GetELFFileBegin() + HeaderLE_.e_shoff);
//		sheader_type* sh_e = sh_b + HeaderLE_.e_shnum;
//
//		SectionHeaders_.resize(HeaderLE_.e_shnum);
//		std::copy( sh_b, sh_e, SectionHeaders_.begin() );
//
//		std::for_each( SectionHeaders_.begin(), SectionHeaders_.end(), ByteSwapHelper<sheader_type>(ELFTraits::sheader_layout) );
//	}
//
//	void _ReadRawProgramData()
//	{
//		std::for_each( ProgramHeaders_.begin(), ProgramHeaders_.end(), 
//			[&]( const pheader_type& p )
//		{ 
//			memrange_t fdata;
//			fdata.data_		= GetELFFileBegin() + p.p_offset;
//			fdata.length_	= p.p_filesz;
//
//			PData_.push_back(fdata);
//		} );
//	}
//
//	void _ReadRawSectionData()
//	{
//		std::for_each( SectionHeaders_.begin(), SectionHeaders_.end(), 
//			[&]( const sheader_type& s )
//		{ 
//			memrange_t fdata;
//			fdata.data_		= GetELFFileBegin() + s.sh_offset;
//			fdata.length_	= s.sh_size;
//
//			SData_.push_back(fdata);
//		} );
//	}
//
//	void _ReadSectionNames()
//	{
//		if ( 0 != HeaderLE_.e_shstrndx )
//		{
//			const char* SectionNameSTBL = (const char*)SData_[HeaderLE_.e_shstrndx].begin();
//
//			std::transform( SectionHeaders_.begin(), SectionHeaders_.end(), std::back_inserter( SHNames_ ), 
//				[SectionNameSTBL]( const sheader_type& sh )->string
//			{
//				return string( SectionNameSTBL + sh.sh_name );
//			});
//		}		
//	}
//
//	void _ReadStringTable()
//	{
//		const size_t strtab_index = std::distance( 
//			SHNames_.begin(), 
//			std::find( SHNames_.begin(), SHNames_.end(), ".strtab" ) );
//
//		if ( strtab_index != SHNames_.size() )
//		{			
//			const char* StringTableBase		= (const char*)SData_[strtab_index].begin();
//			const size_t StringTableLength	= SectionHeaders_[strtab_index].sh_size;
//			const size_t NullTermCharLength = 1;
//
//			for ( size_t BytesRead = 0; BytesRead < StringTableLength; )
//			{
//				string temp( StringTableBase + BytesRead );
//				StringTBL_[BytesRead] = temp;
//				BytesRead += (temp.size() + NullTermCharLength);
//			}
//		}
//	}
//
//	void _ReadSymbolTable()
//	{
//		size_t symtab_index = std::distance( 
//			SHNames_.begin(), 
//			std::find( SHNames_.begin(), SHNames_.end(), ".symtab" ) );
//
//		if ( symtab_index != SHNames_.size() )
//		{
//			std::copy( (symbol_type*)SData_[symtab_index].begin(), 
//				(symbol_type*)SData_[symtab_index].end(), std::back_inserter( Symbols ) );
//
//			std::for_each( Symbols.begin(), Symbols.end(), ByteSwapHelper<symbol_type>(ELFTraits::symbol_layout) );
//		}		
//	}
//};

namespace elf
{
	using namespace std;

	vector<size_t> EnumEmbeddedSPUOffsets( const vector<uint8_t>& Data );

	void HeadersToSystemEndian( void* ELF );

	size_t EntryPointIndex( const void* ELF );

	namespace spu
	{
		vector<uint32_t> LoadExecutable( const void* ELF );
	}
}

#endif __ELF_HELPER_H__