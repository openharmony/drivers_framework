def extract_doxygen_method_params(doxy_str):
    """
    Given a doxygen string for a method, extract parameter descriptions
    """
    doxy_var_desc = {}
    doxy_lines = doxy_str.split("\n")
    last_param_desc = ""
    for doxy_line in doxy_lines:
        if " @param " in doxy_line or " \\param " in doxy_line:
            try:
                # Strip out the param
                doxy_line = doxy_line[doxy_line.find("param ") + 6:]
                (var, desc) = doxy_line.split(" ", 1)
                doxy_var_desc[var] = desc.strip()
                last_param_desc = var
            except Exception as e:
                print(str(e))
                pass
        elif " @return " in doxy_line or " \rreturn " in doxy_line:
            last_param_desc = ""
            # not handled for now
        elif last_param_desc:
            try:
                doxy_line = doxy_line.strip()
                if " " not in doxy_line:
                    last_param_desc = ""
                    continue
                doxy_line = doxy_line[doxy_line.find(" ") + 1:]
                doxy_var_desc[last_param_desc] += " " + doxy_line
            except Exception as e:
                print(str(e))
                pass

    return doxy_var_desc
