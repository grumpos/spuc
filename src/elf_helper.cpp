

#include <cassert>
#include <string>
#include <algorithm>
#include <cstdint>

#include "elf.h"
#include "tools.h"

#include "elf_helper.h"


namespace elf
{
	struct _32_traits_t
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

	const char* _32_traits_t::header_layout	= "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\2\2\4\4\4\4\4\2\2\2\2\2\2";
	const char* _32_traits_t::pheader_layout = "\4\4\4\4\4\4\4\4";
	const char* _32_traits_t::sheader_layout = "\4\4\4\4\4\4\4\4\4\4";
	const char* _32_traits_t::symbol_layout	= "\4\4\4\1\1\2";

	struct _64_traits_t
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

	const char* _64_traits_t::header_layout	= "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\2\2\4\x8\x8\x8\4\2\2\2\2\2\2";
	const char* _64_traits_t::pheader_layout = "\4\4\x8\x8\x8\x8\x8\x8";
	const char* _64_traits_t::sheader_layout = "\4\4\x8\x8\x8\x8\4\4\x8\x8";
	const char* _64_traits_t::symbol_layout	= "\4\1\1\2\x8\x8";

	vector<size_t> EnumEmbeddedSPUOffsets( const vector<uint8_t>& Data )
	{
		const uint8_t ELF_SPU_ARCH_ID = 23;
		const uint32_t ELF_MAGIC = *(uint32_t*)ELFMAG;

		const uint8_t* b = Data.data();
		const uint8_t* e = b + Data.size();

		vector<size_t> ELFOffsets;

		while ( 1 )
		{
			while ( b != e && *b != ELFMAG0 )
				++b;

			if ( b == e )
				break;

			if ( *(uint32_t*)b == ELF_MAGIC )
			{
				const Elf32_Ehdr* eh = (Elf32_Ehdr*)b;

				if ( ELF_SPU_ARCH_ID == ((uint8_t*)&eh->e_machine)[1] )
				{
					ELFOffsets.push_back( b - (const uint8_t*)Data.data() );
				}
			}			

			b += sizeof(Elf32_Ehdr);
		}

		return ELFOffsets;
	}

	template<class ELFTraits>
	void HeaderByteswapperHelper( void* ELF )
	{
		typedef typename ELFTraits::header_type		header_type;
		typedef typename ELFTraits::pheader_type	pheader_type;
		typedef typename ELFTraits::sheader_type	sheader_type;
		typedef typename ELFTraits::symbol_type		symbol_type;

		header_type* h = (header_type*)ELF;

		ReverseBytesInRange( h, ELFTraits::header_layout );

		pheader_type* ph_b = (pheader_type*)((const uint8_t*)ELF + h->e_phoff);
		pheader_type* ph_e = ph_b + h->e_phnum;

		std::for_each( ph_b, ph_e, ByteSwapHelper<pheader_type>(ELFTraits::pheader_layout) );

		sheader_type* sh_b = (sheader_type*)((const uint8_t*)ELF + h->e_shoff);
		sheader_type* sh_e = sh_b + h->e_shnum;

		std::for_each( sh_b, sh_e, ByteSwapHelper<sheader_type>(ELFTraits::sheader_layout) );
	};

	void HeadersToSystemEndian( void* ELF )
	{		
		assert( ELF );
		assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

		if ( ELFCLASS32 == ((const uint8_t*)ELF)[EI_CLASS] )
		{
			HeaderByteswapperHelper<_32_traits_t>( ELF );
		}
		else if ( ELFCLASS64 == ((const uint8_t*)ELF)[EI_CLASS] )
		{
			HeaderByteswapperHelper<_64_traits_t>( ELF );
		}
	}

	size_t EntryPointIndex( const void* ELF )
	{
		assert( ELF );
		assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

		_32_traits_t::header_type* h = (_32_traits_t::header_type*)ELF;

		_32_traits_t::pheader_type* p = (_32_traits_t::pheader_type*)((const uint8_t*)ELF + h->e_phoff);

		return (h->e_entry - p->p_vaddr) / 4;
	}

	namespace spu
	{
		/*void LoadImage( memmap_t* ELF, vector<uint8_t>& Image )
		{

		}*/

		vector<uint32_t> LoadExecutable( const void* ELF )
		{
			assert( ELF );
			assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

			_32_traits_t::header_type* h = (_32_traits_t::header_type*)ELF;			

			_32_traits_t::pheader_type* p = (_32_traits_t::pheader_type*)((const uint8_t*)ELF + h->e_phoff);

			return vector<uint32_t>( 
				(const uint32_t*)((const uint8_t*)ELF + p->p_offset),
				(const uint32_t*)((const uint8_t*)ELF + p->p_offset + p->p_filesz) );
		}
	}
}

