function i = reactornet_hndl(r)
% Integer used to access kernel object
%
% :param r:
%     Instance of class :mat:func:`ReactorNet`
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the kinetics kernel object.
%

i = r.index;
