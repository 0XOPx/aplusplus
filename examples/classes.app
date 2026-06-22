// A++ Classes Example
// Author: OXOP

class Animal {
    func init(name: string, sound: string) {
        self.name = name
        self.sound = sound
    }

    func speak() {
        println(self.name + " says " + self.sound + "!")
    }

    func getName() -> string {
        return self.name
    }
}

class Dog {
    func init(name: string) {
        self.name = name
        self.tricks = []
    }

    func learn(trick: string) {
        push(self.tricks, trick)
        println(self.name + " learned: " + trick)
    }

    func perform() {
        println(self.name + " knows " + str(len(self.tricks)) + " tricks:")
        let i = 0
        while i < len(self.tricks) {
            println("  - " + self.tricks[i])
            i = i + 1
        }
    }
}

class Rectangle {
    func init(width: float, height: float) {
        self.width = width
        self.height = height
    }

    func area() -> float {
        return self.width * self.height
    }

    func perimeter() -> float {
        return 2.0 * (self.width + self.height)
    }

    func describe() {
        println("Rectangle " + str(self.width) + "x" + str(self.height))
        println("  Area: " + str(self.area()))
        println("  Perimeter: " + str(self.perimeter()))
    }
}

// Main program
let cat = Animal("Whiskers", "Meow")
let dog_anim = Animal("Rex", "Woof")
cat.speak()
dog_anim.speak()

println("")

let buddy = Dog("Buddy")
buddy.learn("sit")
buddy.learn("shake")
buddy.learn("roll over")
buddy.perform()

println("")

let rect = Rectangle(5.0, 3.0)
rect.describe()
