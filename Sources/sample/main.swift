import Foundation

// to emit .swiftinterface
//   swiftc -enable-library-evolution -emit-module-interface-path sample.swiftinterface -module-name sample -parse -parse-as-library Sources/sample/main.swift
// to emit binary (with optimization)
//   swiftc -o sample -O Sources/sample/main.swift

public enum SampleEnum: CaseIterable {
    case alpha
    case bravo
    
    var isAlpha: Bool {
        guard case .alpha = self else {
            return false
        }
        return true
    }
    
    func isBravo() -> Bool {
        guard case .bravo = self else {
            return false
        }
        return true
    }
}

public struct SampleStruct: Identifiable {
    public let id: UUID
    public var count: Int = 0
    
    public var transform: AffineTransform
    var sampleEnum: SampleEnum
}

public class SampleClass {
    public let id: String
    public let location: Float
    
    struct SubType {
    }
    
    var sub: SubType
    
    public init(id: String, location: Float) {
        self.id = id
        self.location = location
        self.sub = SubType()
    }
}

public class SampleSubclass: SampleClass {
    var partTwo: SampleStruct
    
    init(partTwo: SampleStruct) {
        self.partTwo = partTwo
        super.init(id: partTwo.id.uuidString, location: 0)
    }
}
