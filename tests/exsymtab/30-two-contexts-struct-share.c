/*
 * Share struct definitions between two contexts.
 */

/* uncomment to enable diagnostic output */
//	#define DIAG(...) diag(__VA_ARGS__)

#include "test_setup.h"

char first_code[] =
"typedef unsigned short __uint16_t, uint16_t;\n"
"typedef unsigned int __uint32_t, uint32_t;\n"
"typedef unsigned long __uint64_t, uint64_t;\n"
"#include <stdlib.h>\n"
"struct point {\n"
"    int x;\n"
"    int yval;\n"
"};\n"
"void * new_point(int x, int newy) {\n"
"    struct point * pt = malloc(sizeof(struct point));\n"
"    pt->x = x;\n"
"    pt->yval = newy;\n"
"    return pt;\n"
"}\n"
;

char second_code[] =
"int sq_distance_to_pt(void * pt_p) {\n"
"    struct point * pt = pt_p;\n"
"    return (pt->x) * (pt->x) + (pt->yval) * (pt->yval);\n"
"}\n"
;

int main(int argc, char **argv) {
	
	/* ---- Compile the first code string and setup the callback data ---- */
	
	TCCState *s1 = tcc_new();
	TokenSym_p* my_symtab;
	setup_and_compile_s1(my_symtab, first_code);
	SETUP_SECOND_CALLBACK_DATA;
	
	/* ---- Allocate a point and manually unpack it ---- */
	void* (*allocate_ptr)(int, int) = tcc_get_symbol(s1, "new_point");
	if (allocate_ptr == NULL) return 1;
	void * point_p = allocate_ptr(6, 9);
	if (point_p == NULL) {
		fail("Unable to allocate point");
		return 1;
	}
	else pass("Allocated point");
	int * manual_unpack = point_p;
	is_i(manual_unpack[0], 6, "manually unpacked x-value");
	is_i(manual_unpack[1], 9, "manually unpacked y-value");
	
	/* ---- Compile the second compiler context ---- */
	TCCState * s_second = tcc_new();
	setup_and_compile_second_state(s_second, second_code);
	int (*sq_dist_ptr)(void*) = tcc_get_symbol(s_second, "sq_distance_to_pt");
	if (sq_dist_ptr == NULL) return 1;
	is_i(sq_dist_ptr(point_p), 9*9+6*6, "Second context able to unpack struct");
	
	/* ---- clean up the memory ---- */
	tcc_delete_extended_symbol_table(my_symtab);
	tcc_delete(s1);
	tcc_delete(s_second);
	free(point_p);
	pass("cleanup");
	
	done_testing();
	
	return 0;
}