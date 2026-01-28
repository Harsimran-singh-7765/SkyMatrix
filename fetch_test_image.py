#!/usr/bin/env python3
import urllib.request
import sys
import os

try:
    from PIL import Image
except ImportError:
    print("Error: Pillow library not found.")
    print("Please install it: pip install Pillow")
    sys.exit(1)

def download_and_convert(url, output_filename="test_image.pgm", size=512):
    print(f"Downloading image from {url}...")
    temp_filename = "temp_download.jpg"
    try:
        # Use a user agent to avoid 403 errors
        req = urllib.request.Request(
            url, 
            data=None, 
            headers={'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.102 Safari/537.36'}
        )
        with urllib.request.urlopen(req) as response, open(temp_filename, 'wb') as out_file:
            out_file.write(response.read())
            
        print("Processing image...")
        img = Image.open(temp_filename)
        
        # Convert to grayscale
        img = img.convert('L')
        
        # Resize if requested
        if size:
            print(f"Resizing to {size}x{size}...")
            img = img.resize((size, size))
            
        # Save as PGM
        img.save(output_filename)
        print(f"Success! Saved to {output_filename}")
        print(f"Now run: ./satellite_analytics --input {output_filename}")
        
        # Cleanup
        os.remove(temp_filename)
        
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    # Robust public domain image (NASA Blue Marble)
    # This is reliable for automated downloading
    SAMPLE_URL = "https://eoimages.gsfc.nasa.gov/images/imagerecords/57000/57752/land_shallow_topo_2048.jpg"
    
    size = 512
    if len(sys.argv) > 1:
        size = int(sys.argv[1])
        
    download_and_convert(SAMPLE_URL, "real_satellite.pgm", size)
