function npts = nPoints(d)
% Number of grid points.
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :return:
%     Integer number of grid points.
%

npts = domain_methods(d.dom_id, 14);
