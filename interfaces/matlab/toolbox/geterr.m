function e = geterr
% Get the error message from a Cantera error
%

try
    e = ctmethods(0,2); % getCanteraError;
catch ME
    e = getReport(ME);
end
