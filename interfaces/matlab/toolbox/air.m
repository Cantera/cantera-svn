function gas = air
% Create an object representing air.
%
% Air is modeled as an ideal gas mixture. The specification is taken
% from file air.cti. Several reactions among oxygen and nitrogen are
% defined.
%
% :return:
%     Instance of class :mat:func:`Solution`
%

gas = importPhase('air.cti','air');
