function n = domain_hndl(d)
% Integer used to access kernel object
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the kinetics kernel object.
%

n = d.dom_id;

