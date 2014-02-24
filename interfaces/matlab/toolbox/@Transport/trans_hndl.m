function n = trans_hndl(a)
% Integer used to access kernel object
%
% :param a:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the transport kernel object.
%

n = a.id;
