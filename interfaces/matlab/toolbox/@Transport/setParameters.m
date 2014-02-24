function setParameters(tr, type, k, p)
% Set parameters of the :mat:func:`Transport` instance.
% Not defined for all transport types.
%
% :param tr:
%     Instance of class :mat:func:`Transport`.
% :param type:
% :param k:
% :param p:
%

v = trans_get(tr.id, 31, type, k, p);
