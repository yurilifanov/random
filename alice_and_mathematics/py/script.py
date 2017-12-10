import numpy as np 
from sympy import binomial
from itertools import combinations

def brute_force(n, m):
  '''
    There are n boolean values, m of them are 1. 
    Take a selection of 0 <= k <= n values, count ones and add 2.
    Do this for all possible selections and take the product.
    Don't try this for high n.
  '''
  assert m <= n, 'n must be greater or equal to m'
  values = m * [1] + (n - m) * [0]
  result = lambda k: 2 + np.array(list(combinations(values, k))).sum(axis=1)
  return np.hstack([result(k).astype(int) for k in range(n + 1)]).prod()

def analytical(n, m):
  '''
    Same as above, but using the analytical formula.
    Don't try this for high n.
  '''
  assert m <= n, 'n must be greater or equal to m'
  result = lambda k: (2 + k) ** (binomial(m, k) * 2 ** (n - m))
  return np.hstack([result(k) for k in range(n + 1)]).prod()