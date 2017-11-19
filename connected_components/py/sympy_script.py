import numpy as np
from sympy import *
from itertools import chain
from sympy.assumptions import assuming, Q

n, m, c = symbols('n m c', real=False)

# n, m and c are positive integers
assumptions = [(Q.integer(x), Q.positive(x)) for x in [n, m, c]]
assumptions = list(chain.from_iterable(assumptions))

def composition(n, k):
	'''
    This is the analytical formula for k-composition, i.e.
    the number of distinct ways to represent a positive integer n
    as a sum of k positive integers. 
    See https://en.wikipedia.org/wiki/Composition_(combinatorics)
  '''
	return binomial(n - Integer(1), k - Integer(1))

with assuming(*assumptions):
	# the full equation
	fun = composition(n - m, c) * (composition(m, c - Integer(1)) \
				+ Integer(2) * composition(m, c) + composition(m, c + Integer(1)))

	# fetch the simplified formula
	fun = simplify(fun)

	# print for use in cpp
	print(fun)

def analytical_formula(N, M, C):
	'''
    Returns the output of the simplified sympy formula,
    converted to integer
  '''
	return int(fun.evalf(subs={n: N, m: M, c: C}))

def simulation(N, M, C):
	'''
    Calculates the answer via brute force enumeration of all
    possible configurations of the N - M marbles
  '''
	cfgs = np.meshgrid(*[np.arange(2) for i in range(N)])
	cfgs = np.vstack([cfg.flatten() for cfg in cfgs]).transpose().astype(int)
	cfgs = cfgs[cfgs.sum(axis=1) == N - M, :]
	diff = np.diff(np.c_[np.zeros(cfgs.shape[0]).astype(int), cfgs], axis=1)
	diff[diff < 0] = 0
	num_conn_comp = diff.sum(axis=1)
	return (num_conn_comp == C).sum()

def comparison(N, M, C):
	'''
    Returns both the analytical and the brute force results
    as tuple
  '''
	return analytical_formula(N, M, C), simulation(N, M, C)