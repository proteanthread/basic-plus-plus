import os

def replace_in_file(filename, old, new):
    with open(filename, 'r') as f:
        text = f.read()
    text = text.replace(old, new)
    with open(filename, 'w') as f:
        f.write(text)

replace_in_file('source/help/help_progmgmt.c', 'HCAT_EDITING', 'HCAT_EDIT')
replace_in_file('source/help/help_testing.c', 'HCAT_DEBUG_TEST', 'HCAT_DEBUG')
replace_in_file('source/help/help_devices.c', 'HCAT_DEVICES_NET', 'HCAT_DEVICE')
