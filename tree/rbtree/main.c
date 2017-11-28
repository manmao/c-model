/* test.c */
#include <stdio.h>
#include <stdlib.h>

#include  "ex-rbtree.h"

int main(int argc, char *argv[])
{
    //c≥ı ºªØ
    struct rb_root mytree =rbtree_init();

    int i, ret, num;
    struct data_type *tmp;

    printf("«Î ‰»Î:");
    scanf("%d",&num);

    printf("Please enter %d integers:\n", num);

    for (i = 0; i < num; i++) {
	tmp = malloc(sizeof(struct data_type));
	if (!tmp)
	    perror("Allocate dynamic memory");
	scanf("%d", &tmp->id);

	ret = rbtree_insert(&mytree, tmp);
        if (ret < 0) {

            fprintf(stderr, "The %d already exists.\n", tmp->id);
            free(tmp);

        }
    }
    printf("\nthe first test\n");
    print_rbtree(&mytree);


    struct data_type tmp1;
    tmp1.id=4;
    rbtree_delete(&mytree,&tmp1);

    printf("\nthe second test\n");
    print_rbtree(&mytree);

    return 0;
}
