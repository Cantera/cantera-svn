function i = xml_hndl(x)
% Integer used to access kernel object
%
% :param x:
%     Instance of class :mat:func:`XML_Node`
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the XML_Node kernel object.
%

i = x.id;
