import numpy as np 
from gmpy import invert
from scipy.special import binom
from scipy.misc import factorial
import pdb

def mesh(maxlist):
  '''
    Constructs a len(maxlist)-dimensional mesh
    with coordinates x[i] in range(0, maxlist[i] + 1)
  '''
  assert all([isinstance(x, int) for x in maxlist]), 'Values must be integers'
  assert np.greater(maxlist, -1).all(), 'Input values must be greater than -1'
  arrays = np.meshgrid(*[np.arange(val + 1) for val in maxlist])
  return np.vstack([arr.flatten() for arr in arrays]).transpose()

def masks(n, m):
  arr = mesh(np.ones(n).astype(int).tolist())
  return arr[arr.sum(1) == m, :].astype(bool)

def count_distinct_stickmen(ds):
  ans = 0
  for m in masks(len(ds), 7).tolist():
    d = ds[m].sum() - 12
    dm = np.delete(ds, np.where(m)[0])
    if d > 0:
      dm = np.r_[dm, d]
    # pdb.set_trace()
    if dm.sum() == 2 * len(dm) - 2:
      g = ds[m]
      for i in range(len(g)):
        for j in range(len(g)):
          if g[i] > 3 and g[j] > 2 and i != j:
            ans += 10
    elif dm.sum() == 0:
      ans += 10
  return ans

def to_array(istr):
  '''
    Converts a string with space separated integers into numpy array
  '''
  return np.array(istr.split()).astype(int)

def num_trees(deg_seq):
  '''
    Formula for the number of trees with given degree sequence
  '''
  if deg_seq.sum() == 2 * len(deg_seq) - 2:
    return factorial(len(deg_seq) - 2) // factorial(deg_seq - 1).prod()
  else:
    return 0

'''
  To count the number of trees with an edge between nodes i and j:
    1. Merge i and j into one node, call it q
    2. Degree d_q of q is: d_q = d_i + d_j - 2
    3. Apply the formula the the new degree sequence
    4. Note that for every tree with set of nodes where i and j are
       replaced by q, there are d_q \choose d_i - 1 = d_q \choose d_j - 1
       possible rewirings of edges which yield a valid tree with i and j
       connected.
'''

def num_stickmen_at_edge(ds, edge, modulo):
  merge_nodes = edge
  di, dj = ds[merge_nodes].tolist()
  d = di + dj - 2

  merged_ds = np.r_[np.delete(ds, merge_nodes), d]
  n_rewirings = binom(d, di - 1)

  assert n_rewirings == binom(d, dj - 1), 'Error in rewirings calc'

  m = int(n_rewirings * num_trees(merged_ds))

  # after some simplifications:
  n = len(ds)
  f = factorial(n - 3)
  g = factorial(ds - 1).prod()
  h = int((f * d) // g)

  assert h == m, 'Simplifications are incorrect'

  # m & h are the number of trees with an edge between merge_nodes
  # n_stickmen = (binom(di, [4, 3]) * binom(dj, [3, 4])).sum()
  n_stickmen = (binom(di - 1, [3, 2]) * binom(dj - 1, [2, 3])).sum()

  return int(n_stickmen * h) % modulo

modulo = 10 ** 9 + 7
ds = to_array('5 4 3 1 1 1 1 1 1 1 1 1 4 1')
ds[::-1].sort()

inv_n_trees = int(invert(int(num_trees(ds)), modulo))

ans = 0
i = 0
while ds[i] > 3:
  j = i + 1
  while ds[j] > 2:
    ans += num_stickmen_at_edge(ds, [i, j], modulo)
    j += 1
  i += 1

print((ans * inv_n_trees) % modulo)
print((count_distinct_stickmen(ds) * inv_n_trees) % modulo)

