struct numbers{
	double a;
	double b;
	char sign;
};

program CALCULATOR_PROG{
	version CALCULATOR_VERS{
		double add(numbers)=1;
		double sub(numbers)=2;
		double mul(numbers)=3;
		double div(numbers)=4;
	}=1;
}=0x31111111;
