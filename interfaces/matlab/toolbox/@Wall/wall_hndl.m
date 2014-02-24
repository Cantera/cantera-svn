function i = wall_hndl(w)
% Integer used to access kernel object
%
% :param w:
%     Instance of class :mat:func:`Wall`
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the wall kernel object.
%

i = w.index;
