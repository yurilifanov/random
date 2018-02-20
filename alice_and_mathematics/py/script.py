import numpy as np 
from sympy import binomial
from itertools import combinations
# from sympy.ntheory.primetest import isprime

def mesh(maxlist):
  '''
    Constructs a len(maxlist)-dimensional mesh
    with coordinates x[i] in range(0, maxlist[i] + 1)
  '''
  assert all([isinstance(x, int) for x in maxlist]), 'Values must be integers'
  assert np.greater(maxlist, -1).all(), 'Input values must be greater than -1'
  arrays = np.meshgrid(*[np.arange(val + 1) for val in maxlist])
  return np.vstack([arr.flatten() for arr in arrays]).transpose()

def brute_force(n, m):
  '''
    There are n boolean values, m of them are 1. 
    Take a selection of 0 <= k <= n values, count ones and add 2.
    Do this for all possible selections and take the product.
    Don't try this for high n.
  '''
  assert m <= n, 'n must be greater or equal to m'
  arr = (np.r_[np.zeros(n - m), np.ones(m)] * mesh(n * [1])).sum(axis=1) + 2
  return arr.astype(int).astype(object).prod() % (10 ** 9 + 7)

def analytical(n, m):
  '''
    Same as above, but using the analytical formula.
    Don't try this for high n.
  '''
  assert m <= n, 'n must be greater or equal to m'
  product = lambda k: (2 + k) ** binomial(m, k)
  result = np.hstack([product(k) for k in range(m + 1)]).astype(object)
  return (result.prod() ** (2 ** (n - m))) % (10 ** 9 + 7)