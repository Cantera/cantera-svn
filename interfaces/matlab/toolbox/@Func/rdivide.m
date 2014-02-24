function r = rdivide(a,b)
% Returns a functor that is the ratio of the input functors
%
% :param a:
%     Instance of class :mat:func:`Func`
% :param b:
%     Instance of class :mat:func:`Func`
% :return:
%     Instance of class :mat:func:`Func`
%
r = Func('ratio',a,b);
