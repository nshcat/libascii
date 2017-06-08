import sys
from jinja2 import Template

def main(argv):
    # We need at least 1 argument, the source file.
    if len(argv) < 2:
        raise Exception('Not enough arguments supplied!')

    # List of all supplied renderer plugin names
    strArgs = argv[2:]
    
    # Create template instance
    template = Template(open(argv[1], 'r').read())
    
    # Render and print out to stdout
    print template.render(plugins=strArgs)
    sys.exit()
    
if __name__ == "__main__":
    main(sys.argv)