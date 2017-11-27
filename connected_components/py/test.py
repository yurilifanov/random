import subprocess
import numpy as np
from sympy import *
from itertools import chain
from sympy.assumptions import assuming, Q

n, m, c = symbols('n m c', real=False)

def composition(n, k):
  '''
    This is the analytical formula for k-composition, i.e.
    the number of distinct ways to represent a positive integer n
    as a sum of k positive integers. 
    See https://en.wikipedia.org/wiki/Composition_(combinatorics)
  '''
  return binomial(n - Integer(1), k - Integer(1))

formula = composition(n - m, c) * (composition(m, c - Integer(1)) \
          + Integer(2) * composition(m, c) + composition(m, c + Integer(1)))

def analytical_formula(N, M, C):
  '''
    Returns the output of the simplified sympy formula,
    converted to integer
  '''
  if (N - M) == 0:
    return 1 if C == 0 else 0
  return int(formula.evalf(subs={n: N, m: M, c: C}))

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

def process_file(file_path):
  '''
    Runs simulations for each test case in input file
  '''
  x = np.fromfile(file_path, dtype=int, sep=' ')[1:].reshape(-1, 3)
  return [(simulation(*y), analytical_formula(*y)) for y in x.tolist()]

if __name__ == '__main__':
  file_path = './data/input.txt'
  test = np.array(process_file(file_path))
  run_str = './a.out < {}'.format(file_path)
  cpp_test = subprocess.check_output(run_str, shell=True)
  cpp_test = np.fromstring(cpp_test, dtype=int, sep='\n')
  assert (test[:, 0] == test[:, 1]).all(), 'Test failed'
  assert (test[:, 0] == cpp_test).all(), 'Cpp test Failed'