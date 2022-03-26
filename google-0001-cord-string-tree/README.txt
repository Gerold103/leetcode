There is a string which is split into a tree. Internal nodes of the tree store length of string parts in the subtree. Leaf nodes store actual data.

For instance, a string '0123456789' could be stored as

                              [len=10]
                                 /\
                     *----------*  *---------*
                    /                         \
                 [len=7]                 [str=789, len=3]
                   /\
         *--------*  *-----*
        /                   \
[str=012, len=3]      [str=3456, len=4]

The tasks are:

1. Design a struct/class which would store the nodes.

2. Make a function which takes such a tree, a position N in the string str stored in the tree, and returns str[N]. Symbol on the given position. For example, in the tree above with N=3 the result is str[3] == '3'.

3. Make a function which takes such a tree, a position N, a length L, and returns a substring of the string str stored in the tree which starts at position N and has the length L. For example, in the tree above with N=3 and L=4 the result is '3456'.

The tree can be assumed to have a small enough depth if that matters.
