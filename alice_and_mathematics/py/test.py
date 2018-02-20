M = 10 ** 9 + 7

A = (M - 1) // 2
B = 2

X = 1
Y = (1 - A) // 2

assert A * X + B * Y == 1, 'Test 1 failed.'

x = 2 * M

a = x % A 
b = x % B

assert (b * X * A + a * Y * B) % (M - 1) == x % (M - 1), 'Test 2 failed.'

from scipy.special import binom
def binom_mod2(n, k):
  N = format(n, '032b')
  K = format(k, '032b')
  J = format(k & (n ^ k), '032b')
  print('{}\n{}\n{}'.format(N, K, J))
  ans = not k & (n ^ k)
  assert ans == bool(int(binom(n, k)) % 2), 'Test 3 failed.'
  return ans