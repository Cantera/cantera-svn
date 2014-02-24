function r = plus(a, b)
% Return a functor representing the sum of two input functors
%
% :param a:
%     Instance of class :mat:func:`Func`
% :param b:
%     Instance of class :mat:func:`Func`
% :return:
%     Instance of class :mat:func:`Func`
%

r = Func('sum',a,b);
