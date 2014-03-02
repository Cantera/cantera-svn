function d = setupGrid(d, grid)
% Set up the solution grid.
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :param grid:
%

domain_methods(d.dom_id, 53, grid);
