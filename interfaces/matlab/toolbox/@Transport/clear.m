function clear(tr)
% Delete the Transport instance.
%
% :param tr:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%

trans_get(tr.id, 0)
