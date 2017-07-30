gb_component(NATIVE gb.eval
    SOURCE_LIST CExpression.c    eval_trans.c
                CHighlight.c     eval_trans_expr.c
                CSystem.c        eval_trans_tree.c
                eval_analyze.c   gb_array.c
                eval.c           gb_error.c
                eval_code.c      gb_table.c
                eval_read.c      main.c
                eval_reserved.c )

target_include_directories(gb.eval PUBLIC 
    ${GAMBAS_SOURCE_ROOT}/main/share/
    ${GAMBAS_SOURCE_ROOT}/main/gbx/
    ${GAMBAS_SOURCE_ROOT}/main/lib/)
