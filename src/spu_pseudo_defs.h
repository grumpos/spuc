//
//#define SPU_PSEUDO_GEN_PROC_RR( OPNAME ) \
//	std::string spu_##OPNAME##_pseudo( SPU_INSTRUCTION op ) \
//	{ \
//	std::string res = OPNAME##_template; \
//	\
//	replace_all( res, "$RT$", "GPR($RT$)" ); \
//	replace_all( res, "$RA$", "GPR($RA$)" ); \
//	replace_all( res, "$RB$", "GPR($RB$)" ); \
//	\
//	replace_all( res, "$RT$", lexical_cast(op.RR.RT) ); \
//	replace_all( res, "$RA$", lexical_cast(op.RR.RA) ); \
//	replace_all( res, "$RB$", lexical_cast(op.RR.RB) ); \
//	\
//	return res; \
//	}
//
//#define SPU_PSEUDO_GEN_PROC_RRR( OPNAME ) \
//	std::string spu_##OPNAME##_pseudo( SPU_INSTRUCTION op ) \
//	{ \
//	std::string res = OPNAME##_template; \
//	\
//	replace_all( res, "$RT$", "GPR($RT$)" ); \
//	replace_all( res, "$RA$", "GPR($RA$)" ); \
//	replace_all( res, "$RB$", "GPR($RB$)" ); \
//	replace_all( res, "$RC$", "GPR($RC$)" ); \
//	\
//	replace_all( res, "$RT$", lexical_cast(op.RRR.RT) ); \
//	replace_all( res, "$RA$", lexical_cast(op.RRR.RA) ); \
//	replace_all( res, "$RB$", lexical_cast(op.RRR.RB) ); \
//	replace_all( res, "$RC$", lexical_cast(op.RRR.RC) ); \
//	\
//	return res; \
//	}
//
//#define SPU_PSEUDO_GEN_PROC_RI7( OPNAME, IMM_CALC ) \
//	std::string spu_##OPNAME##_pseudo( SPU_INSTRUCTION op ) \
//	{ \
//	std::string res = OPNAME##_template; \
//	\
//	replace_all( res, "$RT$", "GPR($RT$)" ); \
//	replace_all( res, "$RA$", "GPR($RA$)" ); \
//	\
//	replace_all( res, "$RT$", lexical_cast(op.RI7.RT) ); \
//	replace_all( res, "$RA$", lexical_cast(op.RI7.RA) ); \
//	replace_all( res, "$I7$", lexical_cast((IMM_CALC)) ); \
//	\
//	return res; \
//	}
//
//#define SPU_PSEUDO_GEN_PROC_RI10( OPNAME, IMM_CALC ) \
//	std::string spu_##OPNAME##_pseudo( SPU_INSTRUCTION op ) \
//	{ \
//	std::string res = OPNAME##_template; \
//	\
//	replace_all( res, "$RT$", "GPR($RT$)" ); \
//	replace_all( res, "$RA$", "GPR($RA$)" ); \
//	\
//	replace_all( res, "$RT$", lexical_cast(op.RI10.RT) ); \
//	replace_all( res, "$RA$", lexical_cast(op.RI10.RA) ); \
//	replace_all( res, "$I10$", lexical_cast((IMM_CALC)) ); \
//	\
//	return res; \
//	}
//
//#define SPU_PSEUDO_GEN_PROC_RI16( OPNAME, IMM_CALC ) \
//	std::string spu_##OPNAME##_pseudo( SPU_INSTRUCTION op ) \
//	{ \
//	std::string res = OPNAME##_template; \
//	\
//	replace_all( res, "$RT$", "GPR($RT$)" ); \
//	\
//	replace_all( res, "$RT$", lexical_cast(op.RI16.RT) ); \
//	replace_all( res, "$I16$", lexical_cast((IMM_CALC)) ); \
//	\
//	return res; \
//	}
//
//#define SPU_PSEUDO_GEN_PROC_RI18( OPNAME, IMM_CALC ) \
//	std::string spu_##OPNAME##_pseudo( SPU_INSTRUCTION op ) \
//	{ \
//	std::string res = OPNAME##_template; \
//	\
//	replace_all( res, "$RT$", "GPR($RT$)" ); \
//	\
//	replace_all( res, "$RT$", lexical_cast(op.RI18.RT) ); \
//	replace_all( res, "$I18$", lexical_cast((IMM_CALC)) ); \
//	\
//	return res; \
//	}