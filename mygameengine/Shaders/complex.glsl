// complex.glsl

struct complex
{
	float real, imag;
};

complex add(complex a, complex b)
{
	return complex(a.real + b.real, a.imag + b.imag);
}

complex mul(complex a, complex b)
{
	return complex(a.real * b.real - a.imag * b.imag, a.real * b.imag + a.imag * b.real);
}

complex conjugate(complex a)
{
	return complex(a.real, -a.imag);
}