__kernel
void matrixMultiplication(__global float* A, __global float* B, __global float* C,  int widthA, int heightA, int widthB)
{
	int i = get_global_id(0);
	int j = get_global_id(1);
	float value=0;


 if (j < heightA && i < widthB)
 {
	for ( int k = 0; k < widthA; k++)
	{
		value += A[k + j * widthA] * B[k*widthB + i];
	}
	C[i + widthB * j] = value;
 }
}


