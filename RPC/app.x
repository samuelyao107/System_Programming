
struct numbers {
    int a;
    int b;
};

program CALCUL_PROG {
    version CALCUL_VERS{
        int ADD(numbers) = 1;
        int SUB(numbers) = 2;
        int MUL(numbers) = 3;
        float DIV(numbers) = 4;
    } = 1;
} = 0x20000001;