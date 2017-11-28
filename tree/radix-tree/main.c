#include "radix-tree.h"

int main(void)
{
    char *test[] = {"abc", "def", "ghi", "jkl", "mno", "pqr", "stu", "vwx", "yz0",
                    "123", "456", "789", "zyx", "wvu", "tsr", "qpo", "nml", "kji"};

    int i = 0;
    int num = sizeof(test)/sizeof(*test);

    printf("num:%d\n", num);

    radix_tree_head *head = radix_tree_head_new();
    radix_tree_initial(head);

    if(!head)
    {
        printf("alloc head failed\n");
    }

    ///插入数据测试
    for(i = 0; i < num; i++)
    {
        radix_tree_insert(head, i, test[i]);
    }


    /// 从指定的index开始，查找固定max_items个数据
    void *result[5];

    radix_tree_gang_lookup(head,result,2,5); //从第二个之后开始找

    for(int i=0;i<6;i++)
    {
        printf("%d :%s\n",i,result[i]);
    }

    ///从指定位置查找数据
    for(i = 0; i < num; i++)
    {
        printf("%s\n",(char*) radix_tree_lookup(head, i));
    }

    for(i = 0; i < num; i++)
    {
        radix_tree_delete(head, i);
    }

    return 0;
}
