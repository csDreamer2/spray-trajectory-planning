#include <iostream>
#include <string>

// OpenCASCADE includes
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <IFSelect_ReturnStatus.hxx>

int main(int argc, char* argv[])
{
    std::cout << "=== OpenCASCADE Configuration Test ===" << std::endl;
    std::cout << std::endl;
    
    // Test 1: Check if OpenCASCADE headers are accessible
    std::cout << "[Test 1] OpenCASCADE headers: OK" << std::endl;
    
    // Test 2: Create STEP reader
    try {
        STEPControl_Reader reader;
        std::cout << "[Test 2] STEPControl_Reader created: OK" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Test 2] FAILED: " << e.what() << std::endl;
        return 1;
    }
    
    // Test 3: Try to load a STEP file if provided
    if (argc > 1) {
        std::string filePath = argv[1];
        std::cout << "[Test 3] Loading STEP file: " << filePath << std::endl;
        
        try {
            STEPControl_Reader reader;
            IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
            
            if (status == IFSelect_RetDone) {
                std::cout << "[Test 3] STEP file read: OK" << std::endl;
                
                reader.TransferRoots();
                TopoDS_Shape shape = reader.OneShape();
                
                if (!shape.IsNull()) {
                    std::cout << "[Test 3] Shape extracted: OK" << std::endl;
                    
                    // Try meshing
                    BRepMesh_IncrementalMesh mesher(shape, 1.0);
                    if (mesher.IsDone()) {
                        std::cout << "[Test 3] Mesh generation: OK" << std::endl;
                    } else {
                        std::cout << "[Test 3] Mesh generation: WARNING (incomplete)" << std::endl;
                    }
                } else {
                    std::cerr << "[Test 3] Shape is NULL" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "[Test 3] FAILED to read STEP file (status: " << status << ")" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Test 3] EXCEPTION: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::cout << "[Test 3] No STEP file provided (skipped)" << std::endl;
        std::cout << "Usage: " << argv[0] << " <path_to_step_file>" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "=== All Tests Passed ===" << std::endl;
    
    return 0;
}
