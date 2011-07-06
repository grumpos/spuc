#include "spu_idb.h"
#include "spu_emu.h"
#include <iostream>
#include <set>
#include <algorithm>

void spu_a( SPU_t* SPU, SPU_INSTRUCTION op );	
void spu_absdb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_addx( SPU_t* SPU, SPU_INSTRUCTION op );	
void spu_ah( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_ahi( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_ai( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_and( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_andbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_andc( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_andhi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_andi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_avgb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bg( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bgx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bihnz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bihz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_binz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bisl( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bisled( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_biz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_br( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_bra( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_brasl( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_brhnz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_brhz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_brnz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_brsl( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_brz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cbd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cbx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cdd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cdx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ceq( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ceqb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ceqbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ceqh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ceqhi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ceqi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cflts( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cfltu( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cg( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgtb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgtbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgth( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgthi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgti( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cgx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_chd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_chx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clgtb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clgtbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clgth( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clgthi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clgti( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_clz( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cntb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_csflt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cuflt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cwd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_cwx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfceq( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfcgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfcmeq( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfcmgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfa( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfm( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfma( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfms( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfnma( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfnms( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dfs( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_dsync( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_eqv( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fa( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fceq( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fcgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fcmeq( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fcmgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fm( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fma( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fms( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fnms( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fs( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fsm( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fsmb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fsmbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_fsmh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_gb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_gbb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_gbh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hbr( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hbra( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hbrr( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_heq( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_heqi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hgti( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hlgt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_hlgti( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_il( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_ila( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_ilh( SPU_t* SPU, SPU_INSTRUCTION op ); 	
void spu_ilhu( SPU_t* SPU, SPU_INSTRUCTION op ); 	
void spu_iohl( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_iret( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_lnop( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_lqa( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_lqd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_lqr( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_lqx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mfspr( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpy( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpya( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyhh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyhha( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyhhau( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyhhu( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpys( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyu( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mpyui( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_mtspr( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_nand( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_nop( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_nor( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_or( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_orbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_orc( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_orhi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_ori( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_orx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rchcnt( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rdch( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rot( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_roth( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rothi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rothm( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rothmi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_roti( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotm( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotma( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotmah( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotmahi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotmai( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotmi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqbii( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqby( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqbybi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqbyi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqmbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqmbii( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqmby( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqmbybi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_rotqmbyi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_selb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_sf( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_sfh( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_sfhi( SPU_t* SPU, SPU_INSTRUCTION op );	
void spu_sfi( SPU_t* SPU, SPU_INSTRUCTION op );		
void spu_sfx( SPU_t* SPU, SPU_INSTRUCTION op );	
void spu_shl( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlhi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shli( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlqbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlqbii( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlqby( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlqbybi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shlqbyi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_shufb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_stop( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_stopd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_stqa( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_stqd( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_stqr( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_stqx( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_sumb( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_sync( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_wrch( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xor( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xorbi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xorhi( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xori( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xsbh( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xshw( SPU_t* SPU, SPU_INSTRUCTION op );
void spu_xswd( SPU_t* SPU, SPU_INSTRUCTION op );

void spu_dummy_solver( struct SPU_t*, union SPU_INSTRUCTION op )
{
	std::cout << "Invalid instruction: " << spu_decode_op_mnemonic( op.Instruction ) << std::endl; 
}

#define SPU_LINK_MNEM_TO_SOLVER(m) spu_mnem_op_t( #m, &spu_##m )

static const spu_mnem_op_t _solvers[] = 
{
	//SPU_LINK_MNEM_TO_SOLVER( a ),
	SPU_LINK_MNEM_TO_SOLVER( absdb ),
	//SPU_LINK_MNEM_TO_SOLVER( addx ),
	//SPU_LINK_MNEM_TO_SOLVER( ah ),
	//SPU_LINK_MNEM_TO_SOLVER( ahi ),
	//SPU_LINK_MNEM_TO_SOLVER( ai ),
	/*SPU_LINK_MNEM_TO_SOLVER( and ),
	SPU_LINK_MNEM_TO_SOLVER( andbi ),
	SPU_LINK_MNEM_TO_SOLVER( andc ),	
	SPU_LINK_MNEM_TO_SOLVER( andhi ),
	SPU_LINK_MNEM_TO_SOLVER( andi ),*/
	SPU_LINK_MNEM_TO_SOLVER( avgb ),
	//SPU_LINK_MNEM_TO_SOLVER( bg ),
	//SPU_LINK_MNEM_TO_SOLVER( bgx ),
	/*SPU_LINK_MNEM_TO_SOLVER( bi ),
	SPU_LINK_MNEM_TO_SOLVER( bihnz ),
	SPU_LINK_MNEM_TO_SOLVER( bihz ),
	SPU_LINK_MNEM_TO_SOLVER( binz ),
	SPU_LINK_MNEM_TO_SOLVER( bisl ),
	SPU_LINK_MNEM_TO_SOLVER( bisled ),
	SPU_LINK_MNEM_TO_SOLVER( biz ),
	SPU_LINK_MNEM_TO_SOLVER( br ),
	SPU_LINK_MNEM_TO_SOLVER( bra ),
	SPU_LINK_MNEM_TO_SOLVER( brasl ),
	SPU_LINK_MNEM_TO_SOLVER( brhnz ),
	SPU_LINK_MNEM_TO_SOLVER( brhz ),
	SPU_LINK_MNEM_TO_SOLVER( brnz ),
	SPU_LINK_MNEM_TO_SOLVER( brsl ),
	SPU_LINK_MNEM_TO_SOLVER( brz ),*/
	SPU_LINK_MNEM_TO_SOLVER( cbd ),
	SPU_LINK_MNEM_TO_SOLVER( cbx ),
	SPU_LINK_MNEM_TO_SOLVER( cdd ),
	SPU_LINK_MNEM_TO_SOLVER( cdx ),
	/*SPU_LINK_MNEM_TO_SOLVER( ceq ),
	SPU_LINK_MNEM_TO_SOLVER( ceqb ),
	SPU_LINK_MNEM_TO_SOLVER( ceqbi ),
	SPU_LINK_MNEM_TO_SOLVER( ceqh ),
	SPU_LINK_MNEM_TO_SOLVER( ceqhi ),
	SPU_LINK_MNEM_TO_SOLVER( ceqi ),*/
	SPU_LINK_MNEM_TO_SOLVER( cflts ),
	SPU_LINK_MNEM_TO_SOLVER( cfltu ),
	//SPU_LINK_MNEM_TO_SOLVER( cg ),
	/*SPU_LINK_MNEM_TO_SOLVER( cgt ),
	SPU_LINK_MNEM_TO_SOLVER( cgtb ),
	SPU_LINK_MNEM_TO_SOLVER( cgtbi ),
	SPU_LINK_MNEM_TO_SOLVER( cgth ),
	SPU_LINK_MNEM_TO_SOLVER( cgthi ),
	SPU_LINK_MNEM_TO_SOLVER( cgti ),*/
	//SPU_LINK_MNEM_TO_SOLVER( cgx ),
	SPU_LINK_MNEM_TO_SOLVER( chd ),
	SPU_LINK_MNEM_TO_SOLVER( chx ),
	/*SPU_LINK_MNEM_TO_SOLVER( clgt ),
	SPU_LINK_MNEM_TO_SOLVER( clgtb ),
	SPU_LINK_MNEM_TO_SOLVER( clgtbi ),
	SPU_LINK_MNEM_TO_SOLVER( clgth ),
	SPU_LINK_MNEM_TO_SOLVER( clgthi ),
	SPU_LINK_MNEM_TO_SOLVER( clgti ),*/
	//SPU_LINK_MNEM_TO_SOLVER( clz ),
	//SPU_LINK_MNEM_TO_SOLVER( cntb ),
	SPU_LINK_MNEM_TO_SOLVER( csflt ),
	SPU_LINK_MNEM_TO_SOLVER( cuflt ),
	SPU_LINK_MNEM_TO_SOLVER( cwd ),
	SPU_LINK_MNEM_TO_SOLVER( cwx ),
	SPU_LINK_MNEM_TO_SOLVER( dfa ),
	SPU_LINK_MNEM_TO_SOLVER( dfceq ),
	SPU_LINK_MNEM_TO_SOLVER( dfcgt ),
	SPU_LINK_MNEM_TO_SOLVER( dfcmeq ),
	SPU_LINK_MNEM_TO_SOLVER( dfcmgt ),
	SPU_LINK_MNEM_TO_SOLVER( dfm ),
	SPU_LINK_MNEM_TO_SOLVER( dfma ),
	SPU_LINK_MNEM_TO_SOLVER( dfms ),
	SPU_LINK_MNEM_TO_SOLVER( dfnma ),
	SPU_LINK_MNEM_TO_SOLVER( dfnms ),
	SPU_LINK_MNEM_TO_SOLVER( dfs ),
	SPU_LINK_MNEM_TO_SOLVER( dsync ),
	SPU_LINK_MNEM_TO_SOLVER( eqv ),
	SPU_LINK_MNEM_TO_SOLVER( fa ),
	SPU_LINK_MNEM_TO_SOLVER( fceq ),
	SPU_LINK_MNEM_TO_SOLVER( fcgt ),
	SPU_LINK_MNEM_TO_SOLVER( fcmeq ),
	SPU_LINK_MNEM_TO_SOLVER( fcmgt ),
	SPU_LINK_MNEM_TO_SOLVER( fm ),
	SPU_LINK_MNEM_TO_SOLVER( fma ),
	SPU_LINK_MNEM_TO_SOLVER( fms ),
	SPU_LINK_MNEM_TO_SOLVER( fnms ),
	SPU_LINK_MNEM_TO_SOLVER( fs ),
	SPU_LINK_MNEM_TO_SOLVER( fsm ),
	SPU_LINK_MNEM_TO_SOLVER( fsmb ),
	//SPU_LINK_MNEM_TO_SOLVER( fsmbi ),
	SPU_LINK_MNEM_TO_SOLVER( fsmh ),
	SPU_LINK_MNEM_TO_SOLVER( gb ),
	SPU_LINK_MNEM_TO_SOLVER( gbb ),
	SPU_LINK_MNEM_TO_SOLVER( gbh ),
	/*SPU_LINK_MNEM_TO_SOLVER( hbr ),
	SPU_LINK_MNEM_TO_SOLVER( hbra ),
	SPU_LINK_MNEM_TO_SOLVER( hbrr ),*/
	/*SPU_LINK_MNEM_TO_SOLVER( heq ),	
	SPU_LINK_MNEM_TO_SOLVER( heqi ),
	SPU_LINK_MNEM_TO_SOLVER( hgt ),	
	SPU_LINK_MNEM_TO_SOLVER( hgti ),
	SPU_LINK_MNEM_TO_SOLVER( hlgt ),
	SPU_LINK_MNEM_TO_SOLVER( hlgti ),
	SPU_LINK_MNEM_TO_SOLVER( il ),
	SPU_LINK_MNEM_TO_SOLVER( ila ),
	SPU_LINK_MNEM_TO_SOLVER( ilh ),
	SPU_LINK_MNEM_TO_SOLVER( ilhu ),
	SPU_LINK_MNEM_TO_SOLVER( iohl ),*/
	//SPU_LINK_MNEM_TO_SOLVER( iret ),
	SPU_LINK_MNEM_TO_SOLVER( lnop ),
	SPU_LINK_MNEM_TO_SOLVER( lqa ),
	SPU_LINK_MNEM_TO_SOLVER( lqd ),
	SPU_LINK_MNEM_TO_SOLVER( lqr ),
	SPU_LINK_MNEM_TO_SOLVER( lqx ),
	SPU_LINK_MNEM_TO_SOLVER( mfspr ),
	/*SPU_LINK_MNEM_TO_SOLVER( mpy ),
	SPU_LINK_MNEM_TO_SOLVER( mpya ),
	SPU_LINK_MNEM_TO_SOLVER( mpyh ),
	SPU_LINK_MNEM_TO_SOLVER( mpyhh ),
	SPU_LINK_MNEM_TO_SOLVER( mpyhha ),
	SPU_LINK_MNEM_TO_SOLVER( mpyhhau ),
	SPU_LINK_MNEM_TO_SOLVER( mpyhhu ),
	SPU_LINK_MNEM_TO_SOLVER( mpyi ),
	SPU_LINK_MNEM_TO_SOLVER( mpys ),
	SPU_LINK_MNEM_TO_SOLVER( mpyu ),
	SPU_LINK_MNEM_TO_SOLVER( mpyui ),*/
	SPU_LINK_MNEM_TO_SOLVER( mtspr ),
	//SPU_LINK_MNEM_TO_SOLVER( nand ),
	SPU_LINK_MNEM_TO_SOLVER( nop ),
	//SPU_LINK_MNEM_TO_SOLVER( nor ),
	/*SPU_LINK_MNEM_TO_SOLVER( or ),
	SPU_LINK_MNEM_TO_SOLVER( orbi ),
	SPU_LINK_MNEM_TO_SOLVER( orc ),
	SPU_LINK_MNEM_TO_SOLVER( orhi ),
	SPU_LINK_MNEM_TO_SOLVER( ori ),
	SPU_LINK_MNEM_TO_SOLVER( orx ),*/
	SPU_LINK_MNEM_TO_SOLVER( rchcnt ),
	SPU_LINK_MNEM_TO_SOLVER( rdch ),
	SPU_LINK_MNEM_TO_SOLVER( rot ),
	SPU_LINK_MNEM_TO_SOLVER( roth ),
	SPU_LINK_MNEM_TO_SOLVER( rothi ),
	SPU_LINK_MNEM_TO_SOLVER( rothm ),
	SPU_LINK_MNEM_TO_SOLVER( rothmi ),
	SPU_LINK_MNEM_TO_SOLVER( roti ),
	SPU_LINK_MNEM_TO_SOLVER( rotm ),
	SPU_LINK_MNEM_TO_SOLVER( rotma ),
	SPU_LINK_MNEM_TO_SOLVER( rotmah ),
	SPU_LINK_MNEM_TO_SOLVER( rotmahi ),
	SPU_LINK_MNEM_TO_SOLVER( rotmai ),
	SPU_LINK_MNEM_TO_SOLVER( rotmi ),
	SPU_LINK_MNEM_TO_SOLVER( rotqbi ),
	SPU_LINK_MNEM_TO_SOLVER( rotqbii ),
	SPU_LINK_MNEM_TO_SOLVER( rotqby ),
	SPU_LINK_MNEM_TO_SOLVER( rotqbybi ),
	SPU_LINK_MNEM_TO_SOLVER( rotqbyi ),
	SPU_LINK_MNEM_TO_SOLVER( rotqmbi ),
	SPU_LINK_MNEM_TO_SOLVER( rotqmbii ),
	SPU_LINK_MNEM_TO_SOLVER( rotqmby ),
	SPU_LINK_MNEM_TO_SOLVER( rotqmbybi ),
	SPU_LINK_MNEM_TO_SOLVER( rotqmbyi ),
	//SPU_LINK_MNEM_TO_SOLVER( selb ),
	/*SPU_LINK_MNEM_TO_SOLVER( sf ),
	SPU_LINK_MNEM_TO_SOLVER( sfh ),
	SPU_LINK_MNEM_TO_SOLVER( sfhi ),
	SPU_LINK_MNEM_TO_SOLVER( sfi ),
	SPU_LINK_MNEM_TO_SOLVER( sfx ),*/
	SPU_LINK_MNEM_TO_SOLVER( shl ),
	SPU_LINK_MNEM_TO_SOLVER( shlh ),
	SPU_LINK_MNEM_TO_SOLVER( shlhi ),
	SPU_LINK_MNEM_TO_SOLVER( shli ),
	SPU_LINK_MNEM_TO_SOLVER( shlqbi ),
	SPU_LINK_MNEM_TO_SOLVER( shlqbii ),
	SPU_LINK_MNEM_TO_SOLVER( shlqby ),
	SPU_LINK_MNEM_TO_SOLVER( shlqbybi ),
	SPU_LINK_MNEM_TO_SOLVER( shlqbyi ),
	SPU_LINK_MNEM_TO_SOLVER( shufb ),
	SPU_LINK_MNEM_TO_SOLVER( stop ),
	SPU_LINK_MNEM_TO_SOLVER( stopd ),
	SPU_LINK_MNEM_TO_SOLVER( stqa ),
	SPU_LINK_MNEM_TO_SOLVER( stqd ),
	SPU_LINK_MNEM_TO_SOLVER( stqr ),
	SPU_LINK_MNEM_TO_SOLVER( stqx ),
	SPU_LINK_MNEM_TO_SOLVER( sumb ),
	SPU_LINK_MNEM_TO_SOLVER( sync ),
	SPU_LINK_MNEM_TO_SOLVER( wrch ),
	/*SPU_LINK_MNEM_TO_SOLVER( xor ),
	SPU_LINK_MNEM_TO_SOLVER( xorbi ),
	SPU_LINK_MNEM_TO_SOLVER( xorhi ),
	SPU_LINK_MNEM_TO_SOLVER( xori ),*/
	SPU_LINK_MNEM_TO_SOLVER( xsbh ),
	SPU_LINK_MNEM_TO_SOLVER( xshw ),
	SPU_LINK_MNEM_TO_SOLVER( xswd ),
};



static std::vector<SPU_OP_TYPE>			db_op_type( SPU_MAX_INSTRUCTION_COUNT, size_t(-1) );
static std::vector<std::string>			db_op_mnemonic( SPU_MAX_INSTRUCTION_COUNT );
static std::vector<SPU_INSTR_PTR>		db_op_solver( SPU_MAX_INSTRUCTION_COUNT, spu_dummy_solver );
std::vector<std::vector<SPU_ARGLIST>>	db_op_arglist( SPU_MAX_INSTRUCTION_COUNT );

/*
struct SPU_IDB
{
	size_t type;
	const char* mnemonic;
};

static const SPU_IDB db_op_ltb[] =
{
#define _A0()        {0xFF,{0xFF,0xFF,0xFF,0}}
#define _A1(a)       {1,{a,0xFF,0xFF,0}}
#define _A2(a,b)     {2,{a,b,0xFF,0}}
#define _A3(a,b,c)   {3,{a,b,c,0}}
#define _A4(a,b,c,d) {4,{a,b,c,d}}

#define APUOP(TAG, FORMAT, OPCODE, MNEMONIC, ASM_FORMAT, DEPENDENCY, PIPE) \
	{ SPU_OP_TYPE_##FORMAT, MNEMONIC },

	APUOP(M_DFCEQ,		RR,	0x3C3,	"dfceq",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFCMEQ,		RR,	0x3CB,	"dfcmeq",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFCGT,		RR,	0x2C3,	"dfcgt",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFCMGT,		RR,	0x2CB,	"dfcmgt",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFTSV,		RI7,0x3BF,	"dftsv",	_A3(A_T,A_A,A_U7),		0x00112,	FPD)

#include "spu-insns.h"

#undef APUOP
};*/

void spu_build_op_db()
{
	
#define _A0()        {0xFF,{0xFF,0xFF,0xFF,0}}
#define _A1(a)       {1,{a,0xFF,0xFF,0}}
#define _A2(a,b)     {2,{a,b,0xFF,0}}
#define _A3(a,b,c)   {3,{a,b,c,0}}
#define _A4(a,b,c,d) {4,{a,b,c,d}}

#define APUOP(TAG,		FORMAT,	OPCODE,	MNEMONIC,	ASM_FORMAT,	DEPENDENCY,	PIPE) \
{ \
	db_op_type[OPCODE] = SPU_OP_TYPE_##FORMAT; \
	db_op_mnemonic[OPCODE] = MNEMONIC; \
	SPU_ARGLIST al = ASM_FORMAT; \
	db_op_arglist[OPCODE].push_back( al ); \
}

#define APUOPFB(TAG,		FORMAT,	OPCODE,	FEATUREBIT, MNEMONIC,	ASM_FORMAT,	DEPENDENCY,	PIPE)/* \
{ \
	db_op_type[OPCODE] = SPU_OP_TYPE_##FORMAT; \
	db_op_mnemonic[OPCODE] = MNEMONIC; \
	SPU_ARGLIST al = ASM_FORMAT; \
	db_op_arglist[OPCODE].push_back( al ); \
}*/

#include "spu-insns.h"
	
}

#undef APUOP
#undef APUOPFB

void link_mnem_to_solver();

size_t spu_decode_op_opcode( uint32_t op )
{
	static bool opcodes_parsed = false;

	if ( !opcodes_parsed )
	{
		spu_build_op_db();
		link_mnem_to_solver();
		opcodes_parsed = true;
	}

	op >>= 21;

	const SPU_OP_TYPE* type_tbl =  db_op_type.data();

	if ( SPU_OP_TYPE_RR == type_tbl[op] || SPU_OP_TYPE_RI7 == type_tbl[op] || SPU_OP_TYPE_LBTI == type_tbl[op] ) 
		return op;
	else if ( SPU_OP_TYPE_RRR == type_tbl[op & 0x780] ) 
		return op & 0x780;
	else if ( SPU_OP_TYPE_RI18 == type_tbl[op & 0x7F0] || SPU_OP_TYPE_LBT == type_tbl[op & 0x7F0] ) 
		return op & 0x7F0;
	else if ( SPU_OP_TYPE_RI10 == type_tbl[op & 0x7F8] ) 
		return op & 0x7F8;
	else if ( SPU_OP_TYPE_RI16 == type_tbl[op & 0x7FC] ) 
		return op & 0x7FC;
	else if ( SPU_OP_TYPE_RI18 == type_tbl[op & 0x7FE] ) 
		return op & 0x7FE;
	else if ( -1 == type_tbl[op] )
		return 0x7FF;

	return op;
}

SPU_OP_TYPE spu_decode_op_type( uint32_t op )
{
	return db_op_type[spu_decode_op_opcode( op )];
}

const char* spu_decode_op_mnemonic( uint32_t op )
{
	return db_op_mnemonic[spu_decode_op_opcode( op )].c_str();
}

SPU_INSTR_PTR spu_decode_op_solver( uint32_t op )
{
	return db_op_solver[spu_decode_op_opcode( op )];
}

SPU_OP_COMPONENTS spu_decode_op_components( uint32_t raw_instr )
{
	const SPU_INSTRUCTION op = (SPU_INSTRUCTION&)raw_instr;
	const SPU_OP_TYPE itype = spu_decode_op_type(raw_instr);

	switch (itype)
	{
	case SPU_OP_TYPE_RRR:
		{
			const SPU_OP_COMPONENTS result = { op.RRR.RT,	op.RRR.RA,	op.RRR.RB,	op.RRR.RC,	0 };
			return result;
		}
	case SPU_OP_TYPE_RR:
		{
			const SPU_OP_COMPONENTS result = { op.RR.RT,	op.RR.RA,	op.RR.RB,	0xFF,		0 };
			return result;
		}
	case SPU_OP_TYPE_RI7:
		{
			const SPU_OP_COMPONENTS result = { op.RI7.RT,	op.RI7.RA,	0xFF,		0xFF,		SignExtend(op.RI7.I7, 7) };
			return result;
		}
	case SPU_OP_TYPE_RI8:
		{
			const SPU_OP_COMPONENTS result = { op.RI8.RT,	op.RI8.RA,	0xFF,		0xFF,		SignExtend(op.RI8.I8, 8) };
			return result;
		}
	case SPU_OP_TYPE_RI10:
		{
			const SPU_OP_COMPONENTS result = { op.RI10.RT,	op.RI10.RA,	0xFF,		0xFF,		SignExtend(op.RI10.I10, 10) };
			return result;
		}
	case SPU_OP_TYPE_RI16:
		{
			const SPU_OP_COMPONENTS result = { op.RI16.RT,	0xFF,		0xFF,		0xFF,		SignExtend(op.RI16.I16, 16) };
			return result;
		}
	case SPU_OP_TYPE_RI18:
		{
			const SPU_OP_COMPONENTS result = { op.RI18.RT,	0xFF,		0xFF,		0xFF,		SignExtend(op.RI18.I18, 18) };
			return result;
		}
	case SPU_OP_TYPE_LBT:
		{
			const uint32_t BRTARG = (uint32_t)SignExtend( op.LBT.I16, 16 );
			const uint32_t BRINST = (uint32_t)SignExtend( ((uint32_t)op.LBT.ROH << 7) | (uint32_t)op.LBT.ROL, 11 );
			const SPU_OP_COMPONENTS result = { 0xFF, 0xFF, 0xFF, 0xFF, (int64_t)(((uint64_t)BRTARG << 32) | (uint64_t)BRINST) };
			return result;
		}
	case SPU_OP_TYPE_LBTI:
		{
			const uint32_t BRTARG = (uint32_t)SignExtend( ((uint32_t)op.LBTI.ROH << 7) | (uint32_t)op.LBTI.ROL, 11 );
			const uint32_t BRINST = 0;
			const SPU_OP_COMPONENTS result = { 0xFF, op.LBTI.RA, 0xFF, 0xFF, (int64_t)(((uint64_t)BRTARG << 32) | (uint64_t)BRINST) };
			return result;
		}
	default:
		{
			const SPU_OP_COMPONENTS result = { 0 };
			return result;
		}
	}
}

void link_mnem_to_solver()
{
	std::set<std::string> unimp;

	size_t i = 0;
	std::for_each( db_op_mnemonic.begin(), db_op_mnemonic.end(), [&unimp, &i]( const std::string& mnem ) {
		if ( !mnem.empty() ) {
			const auto mnem_solver = std::find_if( _solvers, _solvers + _countof(_solvers), [&]( const spu_mnem_op_t& x ) {
				return mnem == x.first;
			} );

			if ( mnem_solver != (_solvers + _countof(_solvers)) ) {
				db_op_solver[i] = mnem_solver->second;
			}
			else {		
				unimp.insert( mnem );
			}
		}

		++i;
	});	

	/*std::ofstream unimp_out("unimp.txt");
	std::copy( unimp.begin(), unimp.end(), std::ostream_iterator<std::string>(unimp_out, "\n") );*/
}