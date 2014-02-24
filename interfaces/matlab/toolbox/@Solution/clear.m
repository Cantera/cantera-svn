function clear(s)
% Delete the kernel objects associated with an instance of
% :mat:func:`Solution`.
%
% :param s:
%     Instance of class :mat:func:`Solution`
%

clear(s.th);
clear(s.kin);
disp('Solution.clear: skipping clearing the transport object... check this!')
%clear(s.tr);
