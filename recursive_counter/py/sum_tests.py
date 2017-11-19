import numpy as np

def mesh(maxlist):
  '''
    Constructs a len(maxlist)-dimensional mesh
    with coordinates x[i] in range(0, maxlist[i] + 1)
  '''
  assert all([isinstance(x, int) for x in maxlist]), 'Values must be integers'
  assert np.greater(maxlist, -1).all(), 'Input values must be greater than -1'
  arrays = np.meshgrid(*[np.arange(val + 1) for val in maxlist])
  return np.vstack([arr.flatten() for arr in arrays]).transpose()

def sum1_slow(m, n):
  '''
    Brute force computes 
    $\sum_{i = 0}^m\sum_{j = 0}^n\min\{i, j\}$
  '''
  return np.sum(np.min(mesh([m, n]), axis=1))

def sum1_fast(m, n):
  '''
    Uses the simplified formula for 
    $\sum_{i = 0}^m\sum_{j = 0}^n\min\{i, j\}$
  '''
  m, n = np.sort([m, n])
  return m * (m + 1) * (3 * n - m + 1) // 6

def sum1_test(xm, xn):
  '''
    Tests the brute force and simplified formula
    against each other for a range of n, m values
    from 0 to xn and xm respectively
  '''
  return np.array([fun(*vals) for fun in [sum1_slow, sum1_fast]
          for vals in mesh([xm, xn]).tolist()]).reshape(2, -1)

# run test of sum 1
assert np.equal(*sum1_test(100, 100).tolist()).all(), 'Sum 1 test failed'