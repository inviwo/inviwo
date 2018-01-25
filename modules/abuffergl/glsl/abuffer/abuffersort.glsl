#ifndef ABUFFER_SORT_GLSL
#define ABUFFER_SORT_GLSL

//Local memory array (L1)
u8vec4 rgba_Array[ABUFFER_SIZE*ABUFFER_DATA_PER_NODE];
float ext_Array[ABUFFER_SIZE*ABUFFER_DATA_PER_NODE];

#define ABUFFER_RGBA_ELEMENT(elem, offset) rgba_Array[((elem)*ABUFFER_DATA_PER_NODE)+offset]
#define ABUFFER_EXT_ELEMENT(elem, offset) ext_Array[((elem)*ABUFFER_DATA_PER_NODE)+offset]

//This is default compare function
//n0, n1 are indices should not be greater than ABUFFER_SIZE
//offset cannot be greater than ABUFFER_DATA_PER_NODE
#define ABUFFER_DATA_COMP_FUNCTION(n0, n1) (ABUFFER_EXT_ELEMENT(n0, 0) > ABUFFER_EXT_ELEMENT(n1, 0))

//Some example compare functions
//#define ABUFFER_DATA_COMP_FUNCTION(n0, n1) (ABUFFER_ELEMENT(n0, 0).x > ABUFFER_ELEMENT(n1, 0).x)
//#define ABUFFER_DATA_COMP_FUNCTION(n0, n1) (ABUFFER_ELEMENT(n0, 1).w > ABUFFER_ELEMENT(n1, 1).w)
//#define ABUFFER_DATA_COMP_FUNCTION(n0, n1) (length(ABUFFER_ELEMENT(n0, 1).xyz) > length(ABUFFER_ELEMENT(n1, 1).xyz))


#define SWAP_FRAGLIST_ITEM(n0, n1) {	for (int sfli=0; sfli<ABUFFER_DATA_PER_NODE; sfli++) {     \
										u8vec4 temp = ABUFFER_RGBA_ELEMENT((n1), sfli);				   \
										ABUFFER_RGBA_ELEMENT((n1), sfli) = ABUFFER_RGBA_ELEMENT((n0), sfli); \
										ABUFFER_RGBA_ELEMENT((n0), sfli) = temp;                        \
									    } \
										for (int sfli=0; sfli<ABUFFER_DATA_PER_NODE; sfli++) {     \
										float temp = ABUFFER_EXT_ELEMENT((n1), sfli);				   \
										ABUFFER_EXT_ELEMENT((n1), sfli) = ABUFFER_EXT_ELEMENT((n0), sfli); \
										ABUFFER_EXT_ELEMENT((n0), sfli) = temp;                        \
									}}

#define INIT_ABUFFER_RGBA_ELEMENTS(initVal) { for(int i=0; i<ABUFFER_SIZE; i++) {							\
												for(int sfli=0; sfli<ABUFFER_DATA_PER_NODE; sfli++) {	    \
													ABUFFER_RGBA_ELEMENT(i, sfli) = initVal;			   	        \
										       }}}

#define INIT_ABUFFER_EXT_ELEMENTS(initVal) { for(int i=0; i<ABUFFER_SIZE; i++) {							\
												for(int sfli=0; sfli<ABUFFER_DATA_PER_NODE; sfli++) {	    \
													ABUFFER_EXT_ELEMENT(i, sfli) = initVal;			   	    \
										       }}}

//Bubble sort
void bubbleSort(int array_size) {  
	if (array_size<=1) return;
	for (int i = (array_size - 2); i >= 0; --i) {
		for (int j = 0; j <= i; ++j) {
			if (ABUFFER_DATA_COMP_FUNCTION(j, j+1)) {
				SWAP_FRAGLIST_ITEM(j, j+1)
			}
		}
	}
}

#endif