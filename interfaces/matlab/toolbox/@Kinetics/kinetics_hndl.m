function i = kinetics_hndl(k)
% Integer used to access kernel object
%
% :param k:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the kinetics kernel object.
%

i = k.id;
