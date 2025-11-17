import sys
# Characters to find
chars_to_find = "HW_"
pol = ["elect","holes"]

# Lists to write
filename_pol = ""
filename_list = []
list_elect = []
list_holes = []
list_org = []

# feb_sw_type
feb_options = ["A", "B"]
feb_type_A = [7,6,5,4,3,2,1,0]
feb_type_B = [1,0,3,2,5,4,7,6]

def open_read_file_list(filename):
    try:
        with open(filename, 'r') as file:
            for line in file:
                filename_list.append(line.strip())
                #print(line.strip())
    except FileNotFoundError:
        print(f"Error: The file '{filename}' was not found")
    except Exception as e:
        print(f"An error ocurred: {e}")
    


def find_pol(s, pol):
    # Find the position of the characters                                                                                                                                                                                                                                      
    position = s.find(pol)
    if position !=-1:
        #print("Filename: ",s.strip())
        return s
    else:
        return None
        
def find_address(s, chars):
    # Find the position of the characters
    position = s.find(chars)
    if position !=-1:
        #slice the string to get the value after the character
        value  = s[position + len(chars):]
        return int(value.split('_')[0].strip())
    
    
def org_files(feb_type, list_pol):
    feb_type_list = []
    list_files = []
    if (feb_type == 'A'):
        feb_type_list.extend(feb_type_A)
    else:
        feb_type_list.extend(feb_type_B)        
    for asic in feb_type_list:
        for my_string in list_pol:
            if (find_address(my_string, chars_to_find) == asic):
                print("HW_address: ", find_address(my_string, chars_to_find), "\t", my_string)
                list_files.append(my_string)
    return list_files

            
def rewrite_plot_file(list_to_write):
    myfile = open("plot.txt", "w")
    for filename in list_to_write:
        myfile.write(filename + '\n')
    myfile.close()
    return 0


def main(module_type):
    feb_type_holes = module_type
    feb_type_elect = feb_options[1] if feb_type_holes == feb_options[0] else feb_options[0] 
    print("Starting the file ordering for a module type: ", module_type)
    # open file containing filenames
    open_read_file_list('plot.txt')
    # Selecting files with elect polarity
    for f in filename_list:
        filename_pol = find_pol(f,'elect')
        if filename_pol!=None:
            list_elect.append(filename_pol)
    # Selecting files with holes polarity
    for f in filename_list:
        filename_pol = find_pol(f,'holes')
        if filename_pol!=None:
            list_holes.append(filename_pol)
            
    # Ordering the files for each polarity
    list_org.extend(org_files(feb_type_elect, list_elect))
    list_org.extend(org_files(feb_type_holes, list_holes))

    rewrite_plot_file(list_org)
    

if __name__ == "__main__":
    if len(sys.argv)>1:
        main(sys.argv[1])
    else:
        print("No argument provided")
