

#include <cassert>
#include <string>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>

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

		for ( ;; )
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

					b += 1;//_byteswap_ulong(eh->e_shoff) + (_byteswap_ulong(eh->e_shnum) * _byteswap_ulong(eh->e_shentsize));
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

	vector<uint32_t> PPUExecutables( void* ELF )
	{
		typedef _64_traits_t::header_type	header_type;
		typedef _64_traits_t::pheader_type	pheader_type;
		typedef _64_traits_t::sheader_type	sheader_type;
		typedef _64_traits_t::symbol_type	symbol_type;

		header_type* h = (header_type*)ELF;

		pheader_type* ph_b = (pheader_type*)((const uint8_t*)ELF + h->e_phoff);
		pheader_type* ph_e = ph_b + h->e_phnum;

		vector<uint32_t> Binary;

		std::for_each( ph_b, ph_e, [&](pheader_type PH)
		{
			if ( (PH.p_type & PT_LOAD) && (PH.p_filesz) )
			{
				const uint32_t* b = (uint32_t*)((uint8_t*)ELF + PH.p_offset);

				size_t NextEntry = Binary.size();

				Binary.resize( Binary.size() + PH.p_filesz/4, 0 );

				memcpy( &Binary[NextEntry], b, PH.p_filesz );
			}
		} );

		/*ofstream off("PPU bindump.txt");
		std::copy( Binary.cbegin(), Binary.cend(), std::ostream_iterator<uint32_t>(off) );*/

		return Binary;
	}

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

	size_t Entry( const void* ELF )
	{
		assert( ELF );
		assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

		if ( ELFCLASS32 == ((char*)ELF)[EI_CLASS] )
		{
			_32_traits_t::header_type* h = (_32_traits_t::header_type*)ELF;

			return h->e_entry;
		}
		else if ( ELFCLASS64 == ((char*)ELF)[EI_CLASS] )
		{
			_64_traits_t::header_type* h = (_64_traits_t::header_type*)ELF;

			return h->e_entry;
		}

		return 0;
	}

	size_t VirtualBaseAddr( const void* ELF )
	{
		assert( ELF );
		assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

		if ( ELFCLASS32 == ((char*)ELF)[EI_CLASS] )
		{
			_32_traits_t::header_type* h = (_32_traits_t::header_type*)ELF;

			_32_traits_t::pheader_type* p = (_32_traits_t::pheader_type*)((const uint8_t*)ELF + h->e_phoff);
			_32_traits_t::pheader_type* p_end = p + h->e_phnum;

			while ( p != p_end )
			{
				if ( p->p_flags & PF_X )
				{
					return p->p_vaddr;
				}
				++p;
			}

			return 0;
		}
		else if ( ELFCLASS64 == ((char*)ELF)[EI_CLASS] )
		{
			_64_traits_t::header_type* h = (_64_traits_t::header_type*)ELF;

			_64_traits_t::pheader_type* p = (_64_traits_t::pheader_type*)((const uint8_t*)ELF + h->e_phoff);

			return p->p_vaddr;
		}

		return 0;
	}

	size_t EntryPointIndex( const void* ELF )
	{
		return (Entry(ELF) - VirtualBaseAddr(ELF)) / 4;
	}

	namespace spu
	{
		static const Elf64_Half MACHINE_PPU = 21;
		static const Elf64_Half MACHINE_SPU = 23;

		vector<uint32_t> LoadExecutable( const void* ELF )
		{
			assert( ELF );
			assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

			if ( MACHINE_PPU == *(Elf64_Half*)((uint8_t*)ELF + 18) )
			{
				_64_traits_t::header_type* h = (_64_traits_t::header_type*)ELF;			

				_64_traits_t::pheader_type* p = (_64_traits_t::pheader_type*)((const uint8_t*)ELF + h->e_phoff);

				assert( p->p_flags & PF_X );

				return vector<uint32_t>( 
					(const uint32_t*)((const uint8_t*)ELF + p->p_offset),
					(const uint32_t*)((const uint8_t*)ELF + p->p_offset + p->p_filesz) );
			}
			else if ( MACHINE_SPU == *(Elf64_Half*)((uint8_t*)ELF + 18) )
			{
				_32_traits_t::header_type* h = (_32_traits_t::header_type*)ELF;			

				_32_traits_t::pheader_type* ph_b = (_32_traits_t::pheader_type*)((const uint8_t*)ELF + h->e_phoff);
				_32_traits_t::pheader_type* ph_e = ph_b + h->e_phnum;
				
				_32_traits_t::sheader_type* sh_b = (_32_traits_t::sheader_type*)((const uint8_t*)ELF + h->e_shoff);
				_32_traits_t::sheader_type* sh_e = sh_b + h->e_shnum;

				vector<uint32_t> Binary;

// 				if ( h->e_shnum )
// 				{
// 					while ( sh_b != sh_e && !(sh_b->sh_flags & SHF_EXECINSTR) )
// 						++sh_b;
// 
// 					if ( sh_b != sh_e )
// 					{
// 						const size_t BaseAddr = sh_b->sh_addr;
// 
// 						std::for_each( sh_b, sh_e, [&](_32_traits_t::sheader_type SH)
// 						{
// 							if ( (SH.sh_type & SHT_PROGBITS) && (SH.sh_size) && (SH.sh_flags & SHF_EXECINSTR) )
// 							{
// 								//size_t NextEntry = Binary.size();
// 
// 								Binary.resize( (SH.sh_addr - BaseAddr + SH.sh_size)/4, 0 );
// 
// 								const uint32_t* b = (uint32_t*)((uint8_t*)ELF + SH.sh_offset);
// 
// 								memcpy( &Binary[(SH.sh_addr - BaseAddr)/4], b, SH.sh_size );
// 							}
// 						} );
// 					}
// 				}
// 				else
				{
					std::for_each( ph_b, ph_e, [&](_32_traits_t::pheader_type PH)
					{
						if ( (PH.p_type & PT_LOAD) && (PH.p_filesz) && (PH.p_flags & PF_X) )
						{
							const uint32_t* b = (uint32_t*)((uint8_t*)ELF + PH.p_offset);

							size_t NextEntry = Binary.size();

							Binary.resize( Binary.size() + PH.p_filesz/4, 0 );

							memcpy( &Binary[NextEntry], b, PH.p_filesz );
						}
					} );
				}				

				return Binary;
			}

			return vector<uint32_t>();
		}

#define ELF32_HEADER	((_32_traits_t::header_type*)ELF)
#define ELF32_PH_BEGIN	(_32_traits_t::pheader_type*)((uint8_t*)ELF + ELF32_HEADER->e_phoff)
#define ELF32_PH_END	((_32_traits_t::pheader_type*)((uint8_t*)ELF + ELF32_HEADER->e_phoff) + ELF32_HEADER->e_phnum)

		void LoadImage( uint8_t* LS, const void* ELF )
		{
			assert( ELF );
			assert( LS );
			assert( *(uint32_t*)ELFMAG == *(uint32_t*)ELF );

			//const _32_traits_t::header_type* h = (const _32_traits_t::header_type*)ELF;

			for ( _32_traits_t::pheader_type* p = ELF32_PH_BEGIN; p != ELF32_PH_END; ++p )
			{
				if ( p->p_type & PT_LOAD )
				{
					memcpy( LS + p->p_vaddr, (const uint8_t*)ELF + p->p_offset, p->p_filesz );

					if ( p->p_memsz > p->p_filesz )
					{
						memset( LS + p->p_vaddr + p->p_filesz, 0, p->p_memsz - p->p_filesz );
					}
				}				
			}
		}
	}
}

